#include "curl_http_client.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <absl/log/absl_log.h>
#include <absl/synchronization/mutex.h>
#include "curl_http_request_handle.h"
#include "http_client.h"
#include "http_client_util.h"

namespace {
// Cleans completed requests and calls the required callbacks.
void ReadCompleteMessages(CurlMultiHandle* multi_handle) {
  CURLMsg* msg;
  int messages_in_queue = 0;
  while ((msg = multi_handle->InfoRead(&messages_in_queue))) {
    if (msg->msg == CURLMSG_DONE) {
      ABSL_LOG(INFO) << CurlEasyHandle::StrError(msg->data.result);
      void* user_data;
      curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &user_data);
      // FCP_CHECK(user_data != nullptr);

      auto handle = static_cast<CurlHttpRequestHandle*>(user_data);
      handle->MarkAsCompleted();
      handle->RemoveFromMulti(multi_handle);
    }
  }
}

// Processes multiple requests while blocked.
absl::Status PerformMultiHandlesBlocked(CurlMultiHandle* multi_handle) {
  int num_running_handles = -1;
  while (num_running_handles) {
    CURLMcode code = multi_handle->Perform(&num_running_handles);
    if (code != CURLM_OK) {
      ABSL_LOG(ERROR) << "MultiPerform failed with code: " << code;
      return absl::InternalError(
          absl::StrCat("MultiPerform failed with code: ", code));
    }

    ReadCompleteMessages(multi_handle);

    if (num_running_handles > 0) {
      code = multi_handle->Poll(/*extra_fds*/ nullptr,
                                /*extra_nfds*/ 0, /*timeout_ms*/ 1000,
                                /*numfds*/ nullptr);
    }
  }

  return absl::OkStatus();
}
}  // namespace

CurlHttpClient::CurlHttpClient(CurlApi* curl_api, std::string test_cert_path)
    : curl_api_(curl_api), test_cert_path_(std::move(test_cert_path)) {
  // FCP_CHECK(curl_api_ != nullptr);
}

std::unique_ptr<HttpRequestHandle> CurlHttpClient::EnqueueRequest(
    std::unique_ptr<HttpRequest> request) {
  ABSL_LOG(INFO) << "Creating a " << ConvertMethodToString(request->method())
                << " request to " << request->uri() << " with body "
                << request->HasBody() << " with headers "
                << request->extra_headers().size();
  for (const auto& [key, value] : request->extra_headers()) {
    ABSL_LOG(INFO) << key << ": " << value;
  }

  return std::make_unique<CurlHttpRequestHandle>(
      std::move(request), curl_api_->CreateEasyHandle(), test_cert_path_);
}

absl::Status CurlHttpClient::PerformRequests(
    std::vector<std::pair<HttpRequestHandle*, HttpRequestCallback*>> requests) {
  ABSL_LOG(INFO) << "PerformRequests";
  std::unique_ptr<CurlMultiHandle> multi_handle =
      curl_api_->CreateMultiHandle();
  // FCP_CHECK(multi_handle != nullptr);

  for (const auto& [request_handle, callback] : requests) {
    // FCP_CHECK(request_handle != nullptr);
    // FCP_CHECK(callback != nullptr);

    auto http_request_handle =
        static_cast<CurlHttpRequestHandle*>(request_handle);

    do {
      absl::Status expr =  http_request_handle->AddToMulti(multi_handle.get(), callback);
      if (expr.code() != absl::StatusCode::kOk) {
        return expr;
      }
    } while (false);
  }

  return PerformMultiHandlesBlocked(multi_handle.get());
}

