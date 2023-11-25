#ifndef BASE_TESTS_UTIL_EVENT_LOOP_HPP
#define BASE_TESTS_UTIL_EVENT_LOOP_HPP

#include <functional>
#include <memory>

#include "function_ref.hpp"

namespace base::util {

struct EventLoop {
  // Returns if the current platform has an event loop implementation
  static bool has_implementation();

  // Returns the main event loop.
  static EventLoop& main();

  // Run the event loop until the given return predicate returns true
  void run_until(util::FunctionRef<bool()> predicate);

  // Schedule execution of the given function on the event loop.
  void perform(util::UniqueFunction<void()>);

  // Run the event loop until all currently pending work has been run.
  void run_pending();

  EventLoop(EventLoop&&) = default;
  EventLoop& operator=(EventLoop&&) = default;
  ~EventLoop();

 private:
  struct Impl;

  EventLoop(std::unique_ptr<Impl>);

  std::unique_ptr<Impl> m_impl;
};

}  // namespace base::util

#endif  // BASE_TESTS_UTIL_EVENT_LOOP_HPP
