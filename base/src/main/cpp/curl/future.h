#pragma once
#include <memory>
#include <optional>
#include <tuple>
#include <variant>

#include <absl/base/macros.h>
#include <absl/synchronization/notification.h>
#include "move_to_lambda.h"
#include "scheduler.h"
#include "unique_value.h"

// Since fcp::Promise is already defined by the reactive streams library
// (fcp/reactive/), we'll define fcp::thread::{Promise, Future}.
namespace thread {

// Forward declarations; see doc comments below
template <typename T>
class Future;
template <typename T>
class Promise;

template <typename T>
struct FuturePair {
  Promise<T> promise;
  Future<T> future;
};

namespace future_internal {
// We want Promise and Future to be created only as a pair, with MakeFuture.
// This type is given permission to construct them.
struct Maker {
  template <typename T>
  static FuturePair<T> Make();
};

// Common state of a Promise / Future pair. Destructed when *both* the promise
// and future are gone.
//
// States: NotSet, Set, Taken
// Transitions:
//   NotSet -> Set: When a value is provided (std::nullopt indicates an
//                  abandoned promise). *Before* ready_ is signalled.
//   Set -> Taken: When a future takes a value. *After* ready_ is signalled.
template <typename T>
class FutureState {
 public:
  bool Wait(absl::Duration timeout) const;
  std::optional<T> Take();
  void Set(std::optional<T> val);

 private:
  enum class State { kNotSet, kSet, kTaken };

  absl::Notification ready_;
  State state_ = State::kNotSet;
  std::optional<T> value_;
};

// A Future and Promise share a single FutureState. That is, FutureState
// is ref-counted, with two initial refs (no additional refs can be created,
// since Future and Promise are move-only). So, we define FutureStateRef as a
// move-only std::shared_ptr.
template <typename T>
using FutureStateRef = UniqueValue<std::shared_ptr<FutureState<T>>>;
}  // namespace future_internal

/**
 * Allows waiting for and retrieving a value (provided eventually by a paired
 * Promise).
 *
 * If the paired Promise is 'abandoned' (destructed without having a value set),
 * then the Future's value is std::nullopt.
 */
template <typename T>
class Future {
 public:
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  /**
   * Retrieves the future value, waiting until it is available.
   * Taking from a future *consumes* it, and so requires an rvalue. To take
   * from a Future<T> f:
   *   std::move(f).Take()
   *
   * If the paired promise is 'abandoned' (destructed before a real value is
   * provided), the value is std::nullopt.
   */
  ABSL_MUST_USE_RESULT
  std::optional<T> Take() && {
    future_internal::FutureStateRef<T> state = std::move(state_);
    // FCP_CHECK(state.has_value());
    return (*state)->Take();
  }

  /**
   * Waits for the value to become available, with a timeout. Unlike Take(),
   * this does *not* consume the value.
   *
   * Returns a bool indicating if the value is available (if so, Take() will
   * return immediately).
   */
  ABSL_MUST_USE_RESULT
  bool Wait(absl::Duration timeout) const {
    // FCP_CHECK(state_.has_value());
    return (*state_)->Wait(timeout);
  }

 private:
  friend struct future_internal::Maker;

  explicit Future(future_internal::FutureStateRef<T> state)
      : state_(std::move(state)) {}

  future_internal::FutureStateRef<T> state_;
};

/**
 * Allows providing a value to satisfy a paired Future.
 *
 * If this Promise is 'abandoned' (destructed without having a value set),
 * then the Future gets the value std::nullopt.
 */
template <typename T>
class Promise {
 public:
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  ~Promise() {
    if (state_.has_value()) {
      // Abandoned
      (*state_)->Set(std::nullopt);
    }
  }

  /**
   * Provides a value to the paired Future. Setting a promise *consumes* it,
   * and so requires an rvalue. To set a Promise<T> p:
   *   std::move(p).Set(...)
   */
  void Set(T value) && {
    future_internal::FutureStateRef<T> state = std::move(state_);
    // FCP_CHECK(state.has_value());
    (*state)->Set(std::move(value));
  }

 private:
  friend struct future_internal::Maker;

  explicit Promise(future_internal::FutureStateRef<T> state)
      : state_(std::move(state)) {}

  future_internal::FutureStateRef<T> state_;
};

/** Creates a paired Future and Promise. */
template <typename T>
FuturePair<T> MakeFuture() {
  return future_internal::Maker::Make<T>();
}

/**
 * Schedules a task which calls a function computing a value. Returns a future
 * to wait for and access the value once it is computed.
 */
template <typename T>
Future<T> ScheduleFuture(Scheduler* scheduler, std::function<T()> func) {
  thread::FuturePair<T> p = thread::MakeFuture<T>();
  MoveToLambdaWrapper<thread::Promise<T>> promise_capture =
      MoveToLambda(std::move(p.promise));
  // Lambda is stateful (since the promise is consumed). This is okay, since
  // it should only be called once.
  scheduler->Schedule([promise_capture, func]() mutable {
    std::move(*promise_capture).Set(func());
  });

  return std::move(p.future);
}

namespace future_internal {

template <typename T>
FuturePair<T> Maker::Make() {
  std::shared_ptr<FutureState<T>> state = std::make_shared<FutureState<T>>();

  auto promise_ref = FutureStateRef<T>(state);
  // Note that we use std::move this time, to avoid ref-count churn.
  auto future_ref = FutureStateRef<T>(std::move(state));
  return {Promise<T>(std::move(promise_ref)), Future<T>(std::move(future_ref))};
}

template <typename T>
bool FutureState<T>::Wait(absl::Duration timeout) const {
  return ready_.WaitForNotificationWithTimeout(timeout);
}

template <typename T>
void FutureState<T>::Set(std::optional<T> val) {
 /* FCP_CHECK(!ready_.HasBeenNotified())
      << "Attempted to set a FutureState which has already been notified";*/
  // Not notified => value_ has *not* been set, and the Promise has exclusive
  // access (no atomics or locks needed below).
  switch (state_) {
    case State::kNotSet:
      state_ = State::kSet;
      value_ = std::move(val);
      // This has release semantics; stores to state_ and value_ will be visible
      // to whomever sees that the notification.
      ready_.Notify();
      return;
    case State::kSet:
      /*FCP_CHECK(false) << "FutureState has been notified, so state_ should be "
                          "kTaken or kSet";*/
      abort();  // Compiler thinks FCP_CHECK(false) might return
    case State::kTaken:
      // FCP_CHECK(false) << "FutureState has already been taken from";
      abort();  // Compiler thinks FCP_CHECK(false) might return
  }
}

template <typename T>
std::optional<T> FutureState<T>::Take() {
  ready_.WaitForNotification();
  // Notified => value_ has been set, and exclusive access has been transferred
  // from the Promise to the Future (no atomics or locks needed below).
  switch (state_) {
    case State::kSet:
      state_ = State::kTaken;
      // value_.has_value() will still be set, but we won't read it again
      // in the kTaken state.
      return std::move(value_);
    case State::kNotSet:
      /*FCP_CHECK(false) << "FutureState has been notified, so state_ should be "
                          "kTaken or kSet";*/
      abort();  // Compiler thinks FCP_CHECK(false) might return
    case State::kTaken:
      // FCP_CHECK(false) << "FutureState has already been taken from";
      abort();  // Compiler thinks FCP_CHECK(false) might return
  }
}

}  // namespace future_internal

}  // namespace thread
