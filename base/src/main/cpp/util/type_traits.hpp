#ifndef BASE_UTIL_TYPE_TRAITS_HPP
#define BASE_UTIL_TYPE_TRAITS_HPP

namespace base::util {
template <typename T>
struct TypeIdentity {
  using type = T;
};

template <typename T>
using type_identity_t = typename TypeIdentity<T>::type;

}  // namespace base::util

#endif  // BASE_UTIL_TYPE_TRAITS_HPP
