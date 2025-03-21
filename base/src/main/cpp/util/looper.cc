#include "looper.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <absl/status/status.h>
#include <absl/strings/string_view.h>

#include "../log_settings.h"
#include "../logging.h"

namespace base {
namespace utils {

ABSL_CONST_INIT thread_local LooperThread* current_looper_thread = nullptr;

LooperThread::LooperThread(absl::string_view name)
    : name_(name),
      started_(false),
      lameduck_(false),
      cleaned_up_(false),
      stopped_(false) {
  thread_ = std::make_unique<std::thread>([this] { this->Loop(); });
}

LooperThread::~LooperThread() {
  // Loop() was called, so Stop() and Join(), which will be no-ops if they've
  // already been called.
  Stop();
  Join();

  if (thread_->joinable()) {
    thread_->join();
  }
}

LooperThread* LooperThread::GetCurrentLooper() { return current_looper_thread; }

// Enqueues the given closure to be run on the looper.
bool LooperThread::Post(std::function<void()>&& runnable) {
  {
    absl::MutexLock l(&mutex_);
    if (lameduck_) {
      BASE_LOG(ERROR) << "Tried to Post to stopped Looper: " << name_;
      return false;
    }
    queue_.emplace_back(runnable);
  }
  queue_changed_.SignalAll();
  return true;
}

void LooperThread::Stop() {
  BASE_LOG(INFO) << "Stop() called for looper: " << name_;
  {
    absl::MutexLock l(&mutex_);
    if (lameduck_) {
      BASE_LOG(INFO) << "Looper already in lame-duck mode: " << name_;
      return;
    }
    BASE_LOG(INFO) << "Looper entering lame-duck mode: " << name_;
    lameduck_ = true;
  }
  queue_changed_.SignalAll();
}

void LooperThread::Join() {
  // Make sure Join is never called from this Looper's own thread.
  if (std::this_thread::get_id() == thread_->get_id()) {
    BASE_LOG(FATAL) << "Join() was called on thread for Looper " << name_;
  }

  BASE_LOG(INFO) << "Join() called for looper: " << name_;
  absl::MutexLock l(&mutex_);
  while (!stopped_) {
    BASE_LOG(INFO) << "Waiting for looper to be stopped: " << name_;
    stopped_changed_.Wait(&mutex_);
  }
  BASE_LOG(INFO) << "Looper is joined: " << name_;
}

std::optional<std::function<void()>> LooperThread::Dequeue() {
  absl::MutexLock l(&mutex_);
  while (queue_.empty()) {
    if (lameduck_) {
      return std::nullopt;
    }
    queue_changed_.Wait(&mutex_);
  }
  auto runnable = std::move(queue_.front());
  queue_.pop_front();
  return runnable;
}

void LooperThread::Loop() {
  current_looper_thread = this;
#ifdef __APPLE__
  // Set a name for the thread to make debugging in Xcode nicer.
  std::string label = "Looper: " + name_;
  pthread_setname_np(label.c_str());
#endif
  {
    absl::MutexLock l(&mutex_);
    if (started_) {
      BASE_LOG(ERROR) << "Looper::Loop() was called more than once for " << name_;
      return;
    }
    started_ = true;
  }
  BASE_LOG(INFO) << "Starting Looper: " << name_;

  while (true) {
    auto maybe_runnable = Dequeue();
    if (!maybe_runnable) {
      // The looper is in lame-duck mode and the queue is empty.
      BASE_LOG(INFO) << "Looper " << name_
                << " is in lameduck mode and the queue is empty. Stopping...";
      break;
    }
    auto runnable = maybe_runnable.value();
    runnable();
  }

  RunAllCleanupHandlers();

  // Mark that the looper is fully stopped.
  {
    absl::MutexLock l(&mutex_);
    stopped_ = true;
  }
  BASE_LOG(INFO) << "Stopped Looper: " << name_;
  stopped_changed_.SignalAll();
  current_looper_thread = nullptr;
}

void LooperThread::AddCleanupHandler(std::function<void()> runnable) {
  absl::MutexLock l(&mutex_);
  if (cleaned_up_) {
    BASE_LOG(ERROR) << "Tried to AddCleanupHandler too late for Looper: " << name_;
    return;
  }
  cleanup_queue_.emplace_back(runnable);
}

std::optional<std::function<void()>> LooperThread::DequeueCleanupHandler() {
  absl::MutexLock l(&mutex_);
  if (!lameduck_) {
    BASE_LOG(FATAL) << "Attempted to dequeue cleanup handler on running looper: "
               << name_;
  }
  if (cleanup_queue_.empty()) {
    cleaned_up_ = true;
    return std::nullopt;
  }
  auto runnable = std::move(cleanup_queue_.front());
  cleanup_queue_.pop_front();
  return runnable;
}

void LooperThread::RunAllCleanupHandlers() {
  BASE_LOG(INFO) << "Running cleanup handlers for looper: " << name_;
  while (true) {
    auto maybe_runnable = DequeueCleanupHandler();
    if (!maybe_runnable) {
      return;
    }
    auto runnable = maybe_runnable.value();
    runnable();
  }
}

}  // namespace utils
}  // namespace base

