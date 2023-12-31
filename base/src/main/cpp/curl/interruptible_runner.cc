#include "interruptible_runner.h"

#include <functional>
#include <utility>
#include <absl/log/absl_log.h>
#include <absl/status/status.h>


absl::Status InterruptibleRunner::Run(std::function<absl::Status()> f,
                                      std::function<void()> abort_function) {
  // Check before even making the call.
  if (should_abort_()) {
    return absl::CancelledError("cancelled before posting callable");
  }
  thread::Future<absl::Status> run_future =
      thread::ScheduleFuture<absl::Status>(thread_pool_.get(), f);
  return WaitUntilDone(std::move(run_future), abort_function);
}

absl::Status InterruptibleRunner::WaitUntilDone(
    thread::Future<absl::Status>&& run_future,
    std::function<void()> abort_function) {
  // Wait until call is done, checking periodically whether we need to abort.
  while (true) {
    if (run_future.Wait(timing_config_.polling_period)) {
      std::optional<absl::Status> future_result = std::move(run_future).Take();
      // std::nullopt indicates the underlying promise was abandoned. To my
      // best knowledge this always indicates a programming error and hence
      // should result in a crash.
      // FCP_CHECK(future_result != std::nullopt);
      return future_result.value();
    }

    if (should_abort_()) {
      return Abort(std::move(run_future), abort_function);
    }
  }
}

absl::Status InterruptibleRunner::Abort(
    thread::Future<absl::Status> run_future,
    std::function<void()> abort_function) {
  ABSL_LOG(WARNING) << "Aborting run.";

  // Attempt to abort the ongoing call.
  abort_function();

  // Wait for at most the graceful shutdown period.
  if (run_future.Wait(timing_config_.graceful_shutdown_period)) {
    // FCP_CHECK(std::move(run_future).Take() != std::nullopt);
    return absl::CancelledError("cancelled after graceful wait");
  }

  // Runnable failed to abort during the graceful shutdown period. Wait for
  // (possibly much) longer, because there's nothing much being
  // gained by returning with TF still running, but resources leak.
  if (run_future.Wait(timing_config_.extended_shutdown_period)) {
    // FCP_CHECK(std::move(run_future).Take() != std::nullopt);
    return absl::CancelledError("cancelled after extended wait");
  }

  // If even waiting for the long period didn't help, exit this process.
  // This is the worst case that will unfortunately happen - we hope the
  // logs above and below make it to a logging backend, allowing to narrow
  // the root cause down to particular models or builds; and the exit(0) should
  // avoid raising a crash dialog when training is running in a background
  // process. Nevertheless the goal should be to never reach this point.
  exit(0);
}

