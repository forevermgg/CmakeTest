#pragma once
#include <functional>
#include <memory>

#include <absl/status/status.h>
#include <absl/time/time.h>
#include "future.h"
#include "scheduler.h"

// An executor that runs operations in a background thread, polling a callback
// periodically whether to abort, and aborting the operation if necessary.
// This uses a single-threaded thread pool. During execution of an operation,
// should_abort is polled periodically (polling_period), and if it returns true,
// the abort_function supplied along with the operation is called. The operation
// is then expected to abort within graceful_shutdown_period. If not, a diag
// code is logged and we wait for some time longer (extended_shutdown_period),
// and if the operation still does not finish, the program exits.
// The destructor blocks until the background thread has become idle.
class InterruptibleRunner {
 public:
  // A struct used to group polling & timeout related parameters.
  struct TimingConfig {
    absl::Duration polling_period;
    absl::Duration graceful_shutdown_period;
    absl::Duration extended_shutdown_period;
  };

  InterruptibleRunner(std::function<bool()> should_abort,
                      const TimingConfig& timing_config)
      : should_abort_(should_abort),
        timing_config_(timing_config) {
    thread_pool_ = CreateThreadPoolScheduler(1);
  }

  ~InterruptibleRunner() { thread_pool_->WaitUntilIdle(); }

  // Executes f() on a background. Returns CANCELLED if the background thread
  // was aborted, or a Status object from the background thread on successful
  // completion.
  absl::Status Run(std::function<absl::Status()> f,
                   std::function<void()> abort_function);

 private:
  absl::Status WaitUntilDone(thread::Future<absl::Status>&& run_future,
                             std::function<void()> abort_function);
  absl::Status Abort(thread::Future<absl::Status> run_future,
                     std::function<void()> abort_function);

  std::unique_ptr<Scheduler> thread_pool_;
  std::function<bool()> should_abort_;
  TimingConfig timing_config_;
};
