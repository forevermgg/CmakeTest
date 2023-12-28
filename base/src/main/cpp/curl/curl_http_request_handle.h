
#include <memory>
#include <string>

#include "absl/synchronization/mutex.h"
#include <curl/curl.h>
#include "curl_api.h"
#include "curl_header_parser.h"
#include "http_client.h"

// A thread-safe curl-based implementation. Designed to be used with
// CurlHttpClient.
class CurlHttpRequestHandle : public HttpRequestHandle {
 public:
  // If non-empty, `test_cert_path` specifies the path to the Certificate
  // Authority (CA) bundle to use instead of the system defaults.
  CurlHttpRequestHandle(std::unique_ptr<HttpRequest> request,
                        std::unique_ptr<CurlEasyHandle> easy_handle,
                        const std::string& test_cert_path);
  ~CurlHttpRequestHandle() override;
  CurlHttpRequestHandle(const CurlHttpRequestHandle&) = delete;
  CurlHttpRequestHandle& operator=(const CurlHttpRequestHandle&) = delete;

  // Adds this request to the corresponding multi-handle that can execute
  // multiple requests in parallel. The corresponding callbacks will be
  // called accordingly.
  absl::Status AddToMulti(CurlMultiHandle* multi_handle,
                          HttpRequestCallback* callback)
      ABSL_LOCKS_EXCLUDED(mutex_);
  // Removes this request from the corresponding multi-handle.
  void RemoveFromMulti(CurlMultiHandle* multi_handle)
      ABSL_LOCKS_EXCLUDED(mutex_);
  // Marks the request as completed which fires the OnComplete callback.
  void MarkAsCompleted() ABSL_LOCKS_EXCLUDED(mutex_);

  // HttpRequestHandle overrides:
  ABSL_MUST_USE_RESULT HttpRequestHandle::SentReceivedBytes
  TotalSentReceivedBytes() const override ABSL_LOCKS_EXCLUDED(mutex_);
  void Cancel() override ABSL_LOCKS_EXCLUDED(mutex_);

 private:
  // Initializes the easy_handle_ in the constructor.
  CURLcode InitializeConnection(const std::string& test_cert_path)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  // Initializes headers from external_headers
  CURLcode InitializeHeaders(const HeaderList& extra_headers,
                             HttpRequest::Method method)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  // A helper function called _sequentially_ when a response header received.
  static size_t HeaderCallback(char* buffer, size_t size, size_t n_items,
                               void* user_data) ABSL_NO_THREAD_SAFETY_ANALYSIS;
  // A helper function called _sequentially_ when a chunk of a body received
  static size_t DownloadCallback(void* body, size_t size, size_t nmemb,
                                 void* user_data)
      ABSL_NO_THREAD_SAFETY_ANALYSIS;
  // A helper function called _sequentially_ to send a chunk of a body.
  static size_t UploadCallback(char* buffer, size_t size, size_t num,
                               void* user_data) ABSL_NO_THREAD_SAFETY_ANALYSIS;

  // Called periodically. Used to cancel the request.
  static size_t ProgressCallback(void* user_data, curl_off_t dltotal,
                                 curl_off_t dlnow, curl_off_t ultotal,
                                 curl_off_t ulnow) ABSL_LOCKS_EXCLUDED(mutex_);

  mutable absl::Mutex mutex_;
  const std::unique_ptr<HttpRequest> request_ ABSL_GUARDED_BY(mutex_);
  std::unique_ptr<HttpResponse> response_ ABSL_GUARDED_BY(mutex_);
  const std::unique_ptr<CurlEasyHandle> easy_handle_ ABSL_GUARDED_BY(mutex_);
  // Used only in the HeaderCallback sequentially.
  CurlHeaderParser header_parser_{};
  // Owned by the caller. Initialized in AddToMulti and then read-only.
  HttpRequestCallback* callback_ ABSL_GUARDED_BY(mutex_);
  bool is_being_performed_ ABSL_GUARDED_BY(mutex_);
  bool is_completed_ ABSL_GUARDED_BY(mutex_);
  bool is_cancelled_ ABSL_GUARDED_BY(mutex_);
  char error_buffer_[CURL_ERROR_SIZE] ABSL_GUARDED_BY(mutex_){};
  // Owned by the class.
  curl_slist* header_list_;
};
