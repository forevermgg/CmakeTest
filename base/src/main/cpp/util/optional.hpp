#pragma once
#ifndef BASE_UTIL_OPTIONAL_HPP
#define BASE_UTIL_OPTIONAL_HPP

#include <optional>
#include <ostream>

namespace base {
namespace util {

template <class T>
using Optional = std::optional<T>;
using None = std::nullopt_t;

template <class T, class... Args>
Optional<T> some(Args&&... args) {
  return std::make_optional<T>(std::forward<Args>(args)...);
}

using std::make_optional;

constexpr auto none = std::nullopt;

template <class T>
struct RemoveOptional {
  using type = T;
};
template <class T>
struct RemoveOptional<Optional<T>> {
  using type = typename RemoveOptional<T>::type;  // Remove recursively
};

/**
 * Writes a T to an ostream, with special handling if T is a std::optional.
 *
 * This function supports both optional and non-optional Ts, so that callers
 * don't need to do their own dispatch.
 */
template <class T>
std::ostream& stream_possible_optional(std::ostream& os, const T& rhs) {
  return os << rhs;
}
template <class T>
std::ostream& stream_possible_optional(std::ostream& os,
                                       const std::optional<T>& rhs) {
  if (rhs) {
    os << "some(" << *rhs << ")";
  } else {
    os << "none";
  }
  return os;
}

}  // namespace util

using util::none;

}  // namespace base

#endif  // BASE_UTIL_OPTIONAL_HPP
