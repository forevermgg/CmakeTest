//
// Created by centforever on 2023/11/25.
//

#ifndef BASE_JSON_SAME_H
#define BASE_JSON_SAME_H

#include <nlohmann/json.hpp>

namespace base {
namespace json {

/// Returns `true` if `a` and `b` are equal.
///
/// Unlike `operator==`, the comparison is non-recursive, and is therefore safe
/// from stack overflow even for deeply nested structures.
///
/// Like `operator==`, two int64_t/uint64_t/double values representing exactly
/// the same number are all considered equal even if their types differ.
///
/// Unlike `operator==`, two `discarded` values are considered equal.
bool JsonSame(const ::nlohmann::json& a, const ::nlohmann::json& b);

}  // namespace json
}  // namespace base

#endif  // BASE_JSON_SAME_H
