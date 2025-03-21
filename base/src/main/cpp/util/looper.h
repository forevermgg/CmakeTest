#ifndef UTILS_LOOPER_H_
#define UTILS_LOOPER_H_

#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>  //NOLINT

#include <absl/status/status.h>
#include <absl/synchronization/mutex.h>

namespace base {
namespace utils {

// A Looper is a queue of closures that runs continuously on a thread until it
// is stopped and joined.
class LooperThread {
 public:
  explicit LooperThread(absl::string_view name);

  ~LooperThread();

  // Enqueues the given closure to be run on the looper.
  // Returns false and logs an error if Stop() has been called.
  bool Post(std::function<void()>&& runnable);

  // Tell the looper to stop accepting new closures, but will continue to run
  // anything already enqueued.
  void Stop();

  // Blocks until the looper is stopped and has run all enqueued closures.
  void Join();

  // Adds a closure to be run right before the underlying thread is joined, to
  // clean up any state associated with the looper. This is run after the looper
  // is stopped, so it cannot enqueue more work on the looper itself.
  void AddCleanupHandler(std::function<void()> runnable);

  // Returns a pointer to the current looper for the calling thread, or nullptr.
  static LooperThread* GetCurrentLooper();

 private:
  // Runs continuously, executing anything added to the queue, until stopped_
  // is true and the queue is empty. Can only be called once.
  void Loop();

  // Returns the next closure on the queue. Will block until one is available.
  // Returns nullopt if the queue is empty and lame_duck_ is true.
  std::optional<std::function<void()>> Dequeue();

  // Returns the next cleanup handler on the queue. Returns nullopt if none.
  std::optional<std::function<void()>> DequeueCleanupHandler();

  // Runs all of the cleanup handler closures that have been added.
  void RunAllCleanupHandlers();

  absl::Mutex mutex_;

  // a human-readable name for the looper.
  std::string name_;

  // the queue of closures to run.
  std::deque<std::function<void()>> queue_ ABSL_GUARDED_BY(mutex_);

  // set to true once Loop is called.
  bool started_ ABSL_GUARDED_BY(mutex_);

  // set to true when the looper should stop accepting new closures.
  bool lameduck_ ABSL_GUARDED_BY(mutex_);

  // set to true when all of the cleanup handlers have completed.
  bool cleaned_up_ ABSL_GUARDED_BY(mutex_);

  // set to true when the looper is in lame-duck mode and has finished running
  // any enqueued loopers.
  bool stopped_ ABSL_GUARDED_BY(mutex_);

  // condition signaled when the queue gets an element or lameduck_ is set.
  absl::CondVar queue_changed_;

  // condition signaled when the queue gets an element or stopped_ is set.
  absl::CondVar stopped_changed_;

  // The thread where the looper runs its closures.
  std::unique_ptr<std::thread> thread_;

  // A list of cleanup closures to run when the thread is joined.
  std::deque<std::function<void()>> cleanup_queue_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace utils
}  // namespace base

#endif  // UTILS_LOOPER_H_
