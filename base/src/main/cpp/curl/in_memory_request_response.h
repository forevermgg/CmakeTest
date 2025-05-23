#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <absl/base/thread_annotations.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/cord.h>
#include <absl/strings/string_view.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include "http_client.h"
#include "interruptible_runner.h"

namespace client {
namespace http {

// Simple `HttpRequest` implementation with an in-memory request body.
class InMemoryHttpRequest : public HttpRequest {
 public:
  // Factory method for creating an instance.
  //
  // Callers are recommended to `std::move` the `body` parameter (or to rely on
  // copy elision), to avoid unnecessary copies of the data.
  //
  // Note that a "Content-Length" header will be constructed automatically, and
  // must not be provided by the caller.
  //
  // If "use_compression" is true, the body will be compressed with
  // gzip. A "Content-Encoding" header will be added, and the "Content-Length"
  // header will be the compressed length.
  //
  // Returns an INVALID_ARGUMENT error if:
  // - the URI is a non-HTTPS URI,
  // - the request has a body but the request method doesn't allow it,
  // - the headers contain a "Content-Length" header.
  static absl::StatusOr<std::unique_ptr<HttpRequest>> Create(
      absl::string_view uri, Method method, HeaderList extra_headers,
      std::string body, bool use_compression);

  absl::string_view uri() const override { return uri_; };
  Method method() const override { return method_; };
  const HeaderList& extra_headers() const override { return headers_; }
  bool HasBody() const override { return !body_.empty(); };

  absl::StatusOr<int64_t> ReadBody(char* buffer, int64_t requested) override;

 private:
  InMemoryHttpRequest(absl::string_view uri, Method method,
                      HeaderList extra_headers, std::string body)
      : uri_(uri),
        method_(method),
        body_(std::move(body)),
        headers_(std::move(extra_headers)) {}

  const std::string uri_;
  const Method method_;
  const std::string body_;
  const HeaderList headers_;
  int64_t cursor_ ABSL_GUARDED_BY(mutex_) = 0;
  mutable absl::Mutex mutex_;
};

// Simple container class for holding an HTTP response code, headers, and
// in-memory request body, as well as metadata for the client-side cache.
struct InMemoryHttpResponse {
  int code;
  // This is empty if no "Content-Encoding" header was present in the response
  // headers.
  std::string content_encoding;
  // This is empty if no "Content-Type" header was present in the response
  // headers.
  std::string content_type;
  absl::Cord body;
};

// Simple `HttpRequestCallback` implementation that stores the response and its
// body in an `InMemoryHttpResponse` object for later consumption.
class InMemoryHttpRequestCallback : public HttpRequestCallback {
 public:
  InMemoryHttpRequestCallback() = default;

  absl::Status OnResponseStarted(const HttpRequest& request,
                                 const HttpResponse& response) override;
  void OnResponseError(const HttpRequest& request,
                       const absl::Status& error) override;
  absl::Status OnResponseBody(const HttpRequest& request,
                              const HttpResponse& response,
                              absl::string_view data) override;
  void OnResponseBodyError(const HttpRequest& request,
                           const HttpResponse& response,
                           const absl::Status& error) override;
  void OnResponseCompleted(const HttpRequest& request,
                           const HttpResponse& response) override;
  absl::StatusOr<InMemoryHttpResponse> Response() const;

 private:
  absl::Status status_ ABSL_GUARDED_BY(mutex_) =
      absl::UnavailableError("No response received");
  std::optional<int> response_code_ ABSL_GUARDED_BY(mutex_);
  std::string content_encoding_ ABSL_GUARDED_BY(mutex_);
  std::string content_type_ ABSL_GUARDED_BY(mutex_);
  std::optional<int64_t> expected_content_length_ ABSL_GUARDED_BY(mutex_);
  absl::Cord response_buffer_ ABSL_GUARDED_BY(mutex_);
  mutable absl::Mutex mutex_;
  std::string client_cache_id_;
};

// Utility for performing a single HTTP request and returning the results (incl.
// the response body) via an in-memory object, in an interruptible way.
//
// If `bytes_received_acc` and `bytes_sent_acc` are non-null then those
// accumulators will also be incremented by the amount of data that was
// received/sent by the request.
//
// Returns an error if the request failed.
absl::StatusOr<InMemoryHttpResponse> PerformRequestInMemory(
    HttpClient& http_client, InterruptibleRunner& interruptible_runner,
    std::unique_ptr<HttpRequest> request, int64_t* bytes_received_acc,
    int64_t* bytes_sent_acc);

// Utility for performing multiple HTTP requests and returning the results
// (incl. the response body) in memory.
//
// Returns an error if issuing the joint `PerformRequests` call failed.
// Otherwise it returns a vector containing the result of each request
// (in the same order the requests were provided in).
absl::StatusOr<std::vector<absl::StatusOr<InMemoryHttpResponse>>>
PerformMultipleRequestsInMemory(
    HttpClient& http_client, InterruptibleRunner& interruptible_runner,
    std::vector<std::unique_ptr<HttpRequest>> requests,
    int64_t* bytes_received_acc, int64_t* bytes_sent_acc);

// Simple class representing a resource for which data is already available
// in-memory (`inline_data`) or for which data needs to be fetched by an HTTP
// GET request (via `uri`). Only one field can ever be set to a non-empty value,
// or both fields may be empty (indicating a zero-length resource for which
// nothing has to be fetched).
class UriOrInlineData {
 public:
  struct InlineData {
    enum class CompressionFormat {
      kUncompressed,
      kGzip,
    };

    absl::Cord data;
    CompressionFormat compression_format = CompressionFormat::kUncompressed;
  };

  struct Uri {
    std::string uri;
    std::string client_cache_id;
    absl::Duration max_age;
  };

  // Creates an instance representing a URI from which data has to be fetched.
  // If the resource represented by the uri should be cached, both
  // `client_cache_id` and `max_age` must be set, otherwise they may be
  // empty/zero.
  static UriOrInlineData CreateUri(std::string uri, std::string client_cache_id,
                                   absl::Duration max_age) {
    return UriOrInlineData({.uri = std::move(uri),
                            .client_cache_id = std::move(client_cache_id),
                            .max_age = max_age},
                           {});
  }
  // Creates an instance representing a resource's already-available (or empty)
  // data.
  static UriOrInlineData CreateInlineData(
      absl::Cord inline_data,
      InlineData::CompressionFormat compression_format) {
    return UriOrInlineData({}, {std::move(inline_data), compression_format});
  }

  const Uri& uri() const { return uri_; }
  const InlineData& inline_data() const { return inline_data_; }

 private:
  UriOrInlineData(Uri uri, InlineData inline_data)
      : uri_(std::move(uri)), inline_data_(std::move(inline_data)) {}

  const Uri uri_;
  const InlineData inline_data_;
};

};  // namespace http
};  // namespace client
