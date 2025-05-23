#pragma once
#include <curl/curl.h>

#include <memory>
#include <string>
#include <type_traits>

#include <absl/synchronization/mutex.h>
// An RAII wrapper around the libcurl easy handle, which works with one request.
// The class is not thread-safe.
class CurlEasyHandle {
 public:
  ~CurlEasyHandle();
  CurlEasyHandle(const CurlEasyHandle&) = delete;
  CurlEasyHandle& operator=(const CurlEasyHandle&) = delete;

  CURLcode GetInfo(CURLINFO info, curl_off_t* value) const;

  template <typename T, typename = std::enable_if_t<std::is_trivial_v<T>>>
  CURLcode SetOpt(CURLoption option, T value) {
    return curl_easy_setopt(easy_handle_, option, value);
  }
  CURLcode SetOpt(CURLoption option, const std::string& value) {
    return SetOpt(option, value.c_str());
  }

  // Converts the curl code into a human-readable form.
  ABSL_MUST_USE_RESULT static std::string StrError(CURLcode code);

  // Returns the underlying curl handle.
  ABSL_MUST_USE_RESULT CURL* GetEasyHandle() const;

 private:
  friend class CurlApi;
  CurlEasyHandle();
  CURL* const easy_handle_;
};

// An RAII wrapper around the libcurl multi handle, which is a bundle of easy
// requests that performs them in parallel in one thread. The class is not
// thread-safe.
class CurlMultiHandle {
 public:
  ~CurlMultiHandle();
  CurlMultiHandle(const CurlMultiHandle&) = delete;
  CurlMultiHandle& operator=(const CurlMultiHandle&) = delete;

  // Fetches the next message in the message queue.
  CURLMsg* InfoRead(int* msgs_in_queue);

  // Add a new request to the bundle.
  CURLMcode AddEasyHandle(CurlEasyHandle* easy_handle);

  // Remove the requests from the bundle. It will cancel an unfinished request,
  // but it will not delete the easy handle.
  CURLMcode RemoveEasyHandle(CurlEasyHandle* easy_handle);

  // Performs all active tasks and returns.
  CURLMcode Perform(int* num_running_handles);

  // Waits for a new job
  CURLMcode Poll(curl_waitfd extra_fds[], unsigned int extra_nfds,
                 int timeout_ms, int* numfds);

  // Converts the curl code into a human-readable form.
  ABSL_MUST_USE_RESULT static std::string StrError(CURLMcode code);

 private:
  friend class CurlApi;
  CurlMultiHandle();
  CURLM* const multi_handle_;
};

// An RAII wrapper around global initialization for libcurl. It forces the user
// to create it first, so the initialization can be made, on which handles
// depend. The class needs to be created only once, and its methods are
// thread-safe.
class CurlApi {
 public:
  CurlApi();
  ~CurlApi();
  CurlApi(const CurlApi&) = delete;
  CurlApi& operator=(const CurlApi&) = delete;

  ABSL_MUST_USE_RESULT std::unique_ptr<CurlEasyHandle> CreateEasyHandle() const;
  ABSL_MUST_USE_RESULT std::unique_ptr<CurlMultiHandle> CreateMultiHandle()
      const;

 private:
  mutable absl::Mutex mutex_;
};