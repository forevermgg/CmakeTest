
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <absl/synchronization/mutex.h>
#include "curl_api.h"
#include "http_client.h"

// A curl-based implementation of the HttpClient interface that uses
// CurlHttpRequestHandle underneath. The implementation assumes that
// CurlHttpClient lives longer than CurlHttpRequestHandle; and
// CurlApi lives longer than CurlHttpClient
class CurlHttpClient : public HttpClient {
 public:
  explicit CurlHttpClient(CurlApi* curl_api, std::string test_cert_path = "");
  ~CurlHttpClient() override = default;
  CurlHttpClient(const CurlHttpClient&) = delete;
  CurlHttpClient& operator=(const CurlHttpClient&) = delete;

  // HttpClient overrides:
  std::unique_ptr<HttpRequestHandle> EnqueueRequest(
      std::unique_ptr<HttpRequest> request) override;

  // Performs the given requests while blocked. Results will be returned to each
  // corresponding `HttpRequestCallback`.
  absl::Status PerformRequests(
      std::vector<std::pair<HttpRequestHandle*, HttpRequestCallback*>> requests)
      override;

 private:
  // Owned by the caller
  const CurlApi* const curl_api_;
  const std::string test_cert_path_;
};
