
#ifndef BASE_UTIL_SCOPE_EXIT_HPP
#define BASE_UTIL_SCOPE_EXIT_HPP

#include <type_traits>
#include <utility>

#include "optional.hpp"

namespace base {
namespace util {

template <class H>
class ScopeExit {
 public:
  explicit ScopeExit(const H& handler) noexcept(
      std::is_nothrow_copy_constructible<H>::value)
      : m_handler(handler) {}

  explicit ScopeExit(H&& handler) noexcept(
      std::is_nothrow_move_constructible<H>::value)
      : m_handler(std::move(handler)) {}

  ScopeExit(ScopeExit&& se) noexcept(
      std::is_nothrow_move_constructible<H>::value)
      : m_handler(std::move(se.m_handler)) {
    se.m_handler = none;
  }

  ~ScopeExit() noexcept {
    if (m_handler) (*m_handler)();
  }

  static_assert(noexcept(std::declval<H>()()),
                "Handler must be nothrow executable");
  static_assert(std::is_nothrow_destructible<H>::value,
                "Handler must be nothrow destructible");

 private:
  util::Optional<H> m_handler;
};

template <class H>
ScopeExit<typename std::remove_reference<H>::type>
make_scope_exit(H&& handler) noexcept(
    noexcept(ScopeExit<typename std::remove_reference<H>::type>(
        std::forward<H>(handler)))) {
  return ScopeExit<typename std::remove_reference<H>::type>(
      std::forward<H>(handler));
}

}  // namespace util
}  // namespace base

#endif  // BASE_UTIL_SCOPE_EXIT_HPP
