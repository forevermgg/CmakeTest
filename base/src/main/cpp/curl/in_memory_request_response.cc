#include "in_memory_request_response.h"

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/ascii.h>
#include <absl/strings/cord.h>
#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/string_view.h>
#include <absl/synchronization/mutex.h>
#include "http_client.h"
#include "http_client_util.h"
#include "interruptible_runner.h"
#include "monitoring.h"

namespace client {
namespace http {

using CompressionFormat =
    client::http::UriOrInlineData::InlineData::CompressionFormat;

static constexpr char kOctetStream[] = "application/octet-stream";
constexpr absl::string_view kClientDecodedGzipSuffix = "+gzip";

absl::StatusOr<std::unique_ptr<HttpRequest>> InMemoryHttpRequest::Create(
    absl::string_view uri, Method method, HeaderList extra_headers,
    std::string body, bool use_compression) {
  // Allow http://localhost:xxxx as an exception to the https-only policy,
  // so that we can use a local http test server.
  if (!absl::StartsWithIgnoreCase(uri, kHttpsScheme) &&
      !absl::StartsWithIgnoreCase(uri, kLocalhostUri)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Non-HTTPS URIs are not supported: ", uri));
  }
  std::optional<std::string> content_length_hdr =
      FindHeader(extra_headers, kContentLengthHdr);
  if (content_length_hdr.has_value()) {
    return absl::InvalidArgumentError(
        "Content-Length header should not be provided!");
  }

  if (!body.empty()) {
    switch (method) {
      case HttpRequest::Method::kPost:
      case HttpRequest::Method::kPatch:
      case HttpRequest::Method::kPut:
      case HttpRequest::Method::kDelete:
        break;
      default:
        return absl::InvalidArgumentError(absl::StrCat(
            "Request method does not allow request body: ", method));
    }
    // Add a Content-Length header, but only if there's a request body.
    extra_headers.push_back({kContentLengthHdr, std::to_string(body.size())});
  }

  return absl::WrapUnique(new InMemoryHttpRequest(
      uri, method, std::move(extra_headers), std::move(body)));
}

absl::StatusOr<int64_t> InMemoryHttpRequest::ReadBody(char* buffer,
                                                      int64_t requested) {
  // This method is called from the HttpClient's thread (we don't really care
  // which one). Hence, we use a mutex to ensure that subsequent calls to this
  // method see the modifications to cursor_.
  absl::WriterMutexLock _(&mutex_);

  // Check whether there's any bytes left to read, and indicate the end has been
  // reached if not.
  int64_t bytes_left = body_.size() - cursor_;
  if (bytes_left == 0) {
    return absl::OutOfRangeError("End of stream reached");
  }
  // FCP_CHECK(buffer != nullptr);
  // FCP_CHECK(requested > 0);
  // Calculate how much data we can return, based on the size of `buffer`.
  int64_t actual_read = bytes_left <= requested ? bytes_left : requested;
  std::memcpy(buffer, body_.data() + cursor_, actual_read);
  cursor_ += actual_read;
  return actual_read;
}

absl::Status InMemoryHttpRequestCallback::OnResponseStarted(
    const HttpRequest& request, const HttpResponse& response) {
  absl::WriterMutexLock _(&mutex_);
  response_code_ = response.code();

  std::optional<std::string> content_encoding_header =
      FindHeader(response.headers(), kContentEncodingHdr);
  if (content_encoding_header.has_value()) {
    // We don't expect the response body to be "Content-Encoding" encoded,
    // because the `HttpClient` is supposed to transparently handle the decoding
    // for us (unless we specified a "Accept-Encoding" header in the request,
    // which would indicate that we wanted to handle the response decoding).
    if (!FindHeader(request.extra_headers(), kAcceptEncodingHdr).has_value()) {
      // Note: technically, we should only receive Content-Encoding values that
      // match the Accept-Encoding values provided in the request headers. The
      // check above isn't quite that strict, but that's probably fine (since
      // such issues should be rare, and can be handled farther up the stack).
      status_ = absl::InvalidArgumentError(
          absl::StrCat("Unexpected header: ", kContentEncodingHdr));
      return status_;
    }
    content_encoding_ = *content_encoding_header;
  }

  content_type_ = FindHeader(response.headers(), kContentTypeHdr).value_or("");

  // Similarly, we should under no circumstances receive a non-identity
  // Transfer-Encoding header, since the `HttpClient` is unconditionally
  // required to undo any such encoding for us.
  std::optional<std::string> transfer_encoding_header =
      FindHeader(response.headers(), kTransferEncodingHdr);
  if (transfer_encoding_header.has_value() &&
      absl::AsciiStrToLower(*transfer_encoding_header) !=
          kIdentityEncodingHdrValue) {
    status_ = absl::InvalidArgumentError(
        absl::StrCat("Unexpected header: ", kTransferEncodingHdr));
    return status_;
  }

  // If no Content-Length header is provided, this means that the server either
  // didn't provide one and is streaming the response, or that the HttpClient
  // implementation transparently decompressed the data for us and stripped the
  // Content-Length header (as per the HttpClient contract).
  std::optional<std::string> content_length_hdr =
      FindHeader(response.headers(), kContentLengthHdr);
  if (!content_length_hdr.has_value()) {
    return absl::OkStatus();
  }

  // A Content-Length header available. Let's parse it so that we know how much
  // data to expect.
  int64_t content_length;
  // Note that SimpleAtoi safely handles non-ASCII data.
  if (!absl::SimpleAtoi(*content_length_hdr, &content_length)) {
    status_ = absl::InvalidArgumentError(
        "Could not parse Content-Length response header");
    return status_;
  }
  if (content_length < 0) {
    status_ = absl::OutOfRangeError(absl::StrCat(
        "Invalid Content-Length response header: ", content_length));
    return status_;
  }
  expected_content_length_ = content_length;

  return absl::OkStatus();
}

void InMemoryHttpRequestCallback::OnResponseError(const HttpRequest& request,
                                                  const absl::Status& error) {
  absl::WriterMutexLock _(&mutex_);
  status_ = absl::Status(
      error.code(), absl::StrCat("Error receiving response headers (error: ",
                                 error.message(), ")"));
}

absl::Status InMemoryHttpRequestCallback::OnResponseBody(
    const HttpRequest& request, const HttpResponse& response,
    absl::string_view data) {
  // This runs on a thread chosen by the HttpClient implementation (i.e. it
  // could be our original thread, or a different one). Ensure that if
  // subsequent callbacks occur on different threads each thread sees the
  // previous threads' updates to response_buffer_.
  absl::WriterMutexLock _(&mutex_);

  // Ensure we're not receiving more data than expected.
  if (expected_content_length_.has_value() &&
      response_buffer_.size() + data.size() > *expected_content_length_) {
    status_ = absl::OutOfRangeError(absl::StrCat(
        "Too much response body data received (rcvd: ", response_buffer_.size(),
        ", new: ", data.size(), ", max: ", *expected_content_length_, ")"));
    return status_;
  }

  // Copy the data into the target buffer. Note that this means we'll always
  // store the response body as a number of memory fragments (rather than a
  // contiguous buffer). However, because HttpClient implementations are
  // encouraged to return response data in fairly large chunks, we don't expect
  // this too cause much overhead.
  response_buffer_.Append(data);

  return absl::OkStatus();
}

void InMemoryHttpRequestCallback::OnResponseBodyError(
    const HttpRequest& request, const HttpResponse& response,
    const absl::Status& error) {
  absl::WriterMutexLock _(&mutex_);
  status_ = absl::Status(
      error.code(),
      absl::StrCat("Error receiving response body (response code: ",
                   response.code(), ", error: ", error.message(), ")"));
}

void InMemoryHttpRequestCallback::OnResponseCompleted(
    const HttpRequest& request, const HttpResponse& response) {
  // Once the body has been received correctly, turn the response code into a
  // canonical code.
  absl::WriterMutexLock _(&mutex_);
  // Note: the case when too *much* response data is unexpectedly received is
  // handled in OnResponseBody (while this handles the case of too little data).
  if (expected_content_length_.has_value() &&
      response_buffer_.size() != *expected_content_length_) {
    status_ = absl::InvalidArgumentError(
        absl::StrCat("Too little response body data received (rcvd: ",
                     response_buffer_.size(),
                     ", expected: ", *expected_content_length_, ")"));
    return;
  }

  status_ = ConvertHttpCodeToStatus(*response_code_);
}

absl::StatusOr<InMemoryHttpResponse> InMemoryHttpRequestCallback::Response()
    const {
  absl::ReaderMutexLock _(&mutex_);
  FCP_RETURN_IF_ERROR(status_);
  // If status_ is OK, then response_code_ and response_headers_ are guaranteed
  // to have values.

  return InMemoryHttpResponse{*response_code_, content_encoding_, content_type_,
                              response_buffer_};
}

absl::StatusOr<InMemoryHttpResponse> PerformRequestInMemory(
    HttpClient& http_client, InterruptibleRunner& interruptible_runner,
    std::unique_ptr<HttpRequest> request, int64_t* bytes_received_acc,
    int64_t* bytes_sent_acc) {
  // Note: we must explicitly instantiate a vector here as opposed to passing an
  // initializer list to PerformRequestsInMemory, because initializer lists do
  // not support move-only values.
  std::vector<std::unique_ptr<HttpRequest>> requests;
  requests.push_back(std::move(request));
  FCP_ASSIGN_OR_RETURN(
      auto result, PerformMultipleRequestsInMemory(
                       http_client, interruptible_runner, std::move(requests),
                       bytes_received_acc, bytes_sent_acc));
  return std::move(result[0]);
}

absl::StatusOr<std::vector<absl::StatusOr<InMemoryHttpResponse>>>
PerformMultipleRequestsInMemory(
    HttpClient& http_client, InterruptibleRunner& interruptible_runner,
    std::vector<std::unique_ptr<HttpRequest>> requests,
    int64_t* bytes_received_acc, int64_t* bytes_sent_acc) {
  // A vector that will own the request handles and callbacks (and will
  // determine their lifetimes).
  std::vector<std::pair<std::unique_ptr<HttpRequestHandle>,
                        std::unique_ptr<InMemoryHttpRequestCallback>>>
      handles_and_callbacks;
  handles_and_callbacks.reserve(requests.size());

  // An accompanying vector that contains just the raw pointers, for passing to
  // `HttpClient::PerformRequests`.
  std::vector<std::pair<HttpRequestHandle*, HttpRequestCallback*>>
      handles_and_callbacks_ptrs;
  handles_and_callbacks_ptrs.reserve(requests.size());

  // Enqueue each request, and create a simple callback for each request which
  // will simply buffer the response body in-memory and allow us to consume that
  // buffer once all requests have finished.
  for (std::unique_ptr<HttpRequest>& request : requests) {
    std::unique_ptr<HttpRequestHandle> handle =
        http_client.EnqueueRequest(std::move(request));
    auto callback = std::make_unique<InMemoryHttpRequestCallback>();
    handles_and_callbacks_ptrs.push_back({handle.get(), callback.get()});
    handles_and_callbacks.push_back({std::move(handle), std::move(callback)});
  }

  // Issue the requests in one call (allowing the HttpClient to issue them
  // concurrently), in an interruptible fashion.
  absl::Status result = interruptible_runner.Run(
      [&http_client, &handles_and_callbacks_ptrs]() {
        return http_client.PerformRequests(handles_and_callbacks_ptrs);
      },
      [&handles_and_callbacks_ptrs] {
        // If we get aborted then call HttpRequestHandle::Cancel on all handles.
        // This should result in the PerformRequests call returning early and
        // InterruptibleRunner::Run returning CANCELLED.
        for (auto [handle, callback] : handles_and_callbacks_ptrs) {
          handle->Cancel();
        }
      });
  // Update the network stats *before* we return (just in case a failed
  // `PerformRequests` call caused some network traffic to have been sent
  // anyway).
  for (auto& [handle, callback] : handles_and_callbacks) {
    HttpRequestHandle::SentReceivedBytes sent_received_bytes =
        handle->TotalSentReceivedBytes();
    if (bytes_received_acc != nullptr) {
      *bytes_received_acc += sent_received_bytes.received_bytes;
    }
    if (bytes_sent_acc != nullptr) {
      *bytes_sent_acc += sent_received_bytes.sent_bytes;
    }
  }

  FCP_RETURN_IF_ERROR(result);

  // Gather and return the results.
  std::vector<absl::StatusOr<InMemoryHttpResponse>> results;
  results.reserve(handles_and_callbacks.size());
  for (auto& [handle, callback] : handles_and_callbacks) {
    results.push_back(callback->Response());
  }
  return results;
}
}  // namespace http
}  // namespace client
