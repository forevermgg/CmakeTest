#include <jni.h>
#include <openssl/crypto.h>
#include <absl/strings/cord.h>
#include <memory>
#include <string>

#include "absl/synchronization/blocking_counter.h"
#include "curl/curl_http_client.h"
#include "curl/http_client.h"
#include "curl/in_memory_request_response.h"
#include "json/JSON.h"
#include "log_settings.h"
#include "logging.h"

absl::Status testNormalWithAbortCheckButNoAbort() {
  int should_abort_calls = 0;
  int abort_function_calls = 0;
  absl::BlockingCounter counter_should_abort(1);
  absl::BlockingCounter counter_did_abort(1);
  std::function<bool()> should_abort =
      [&should_abort_calls, &counter_should_abort, &counter_did_abort]() {
        should_abort_calls++;
        if (should_abort_calls == 2) {
          counter_should_abort.DecrementCount();
          counter_did_abort.Wait();
        }
        return false;
      };
  std::function<void()> abort_function = [&abort_function_calls]() {
    abort_function_calls++;
  };
  InterruptibleRunner interruptibleRunner(
      should_abort, InterruptibleRunner::TimingConfig{
                        .polling_period = absl::ZeroDuration(),
                        .graceful_shutdown_period = absl::InfiniteDuration(),
                        .extended_shutdown_period = absl::InfiniteDuration()});
  absl::Status status = interruptibleRunner.Run(
      [&counter_should_abort, &counter_did_abort]() {
        // Block until should_abort has been called.
        counter_should_abort.Wait();
        // Tell should_abort to return false.
        counter_did_abort.DecrementCount();
        return absl::OkStatus();
      },
      abort_function);
  /*status = interruptibleRunner.Run([]() { return absl::DataLossError(""); },
                                   abort_function);*/
  return status;
}

std::unique_ptr<HttpClient> CreateHttpClient() {
  auto curl_api = std::make_unique<CurlApi>();
  return std::make_unique<CurlHttpClient>(curl_api.get());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_mgg_base_NativeLib_stringFromJNI(JNIEnv* env, jobject /* this */) {
  InterruptibleRunner::TimingConfig timing_config = {
      .polling_period = absl::Milliseconds(1000),
      .graceful_shutdown_period = absl::Milliseconds(1000),
      .extended_shutdown_period = absl::Milliseconds(2000),
  };
  auto should_abort_protocol_callback = []() -> bool {
    /*// Return the Status if failed, or the negated value if successful.
    auto current_time = absl::Now();
    auto condition_polling_period = timing_config.polling_period;*/
    return false;
  };
  absl::StatusOr<std::unique_ptr<HttpRequest>> request =
      client::http::InMemoryHttpRequest::Create(
          "https://dog.ceo/api/breeds/image/random", HttpRequest::Method::kGet,
          {}, "",
          /*use_compression=*/false);
  auto interruptible_runner = std::make_unique<InterruptibleRunner>(
      should_abort_protocol_callback, timing_config);
  std::unique_ptr<HttpClient> http_client = CreateHttpClient();
  HttpClient& http_client_ref = *http_client.get();
  int64_t bytes_received = 0;
  int64_t bytes_sent = 0;
  std::vector<std::unique_ptr<HttpRequest>> requests;
  requests.push_back(*std::move(request));
  absl::StatusOr<
      std::vector<absl::StatusOr<client::http::InMemoryHttpResponse>>>
      results = client::http::PerformMultipleRequestsInMemory(
          http_client_ref, *interruptible_runner.get(), std::move(requests),
          // We pass in non-null pointers for the network
          // stats, to ensure they are correctly updated.
          &bytes_received, &bytes_sent);
  std::string hello = StatusCodeToString(results.status().code());
  if (results.ok()) {
    for (const auto& response : results.value()) {
      if (response.ok()) {
        const client::http::InMemoryHttpResponse& http_response =
            response.value();
        // 处理成功的响应
        hello = "Response code: " + std::string(http_response.body.TryFlat().value_or(""));
      } else {
        const absl::Status& status = response.status();
        // 处理失败的响应
        hello = "Error: ";
      }
    }
  }
  BASE_LOG(ERROR) << "StatusCodeToString = " << hello;
  return env->NewStringUTF(hello.c_str());
}