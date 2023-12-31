#pragma once
/**
* Macro which allows to check for a Status (or StatusOr) and return from the
* current method if not OK. Example:
*
*     Status DoSomething() {
    *       FCP_RETURN_IF_ERROR(Step1());
    *       FCP_RETURN_IF_ERROR(Step2ReturningStatusOr().status());
    *       return FCP_STATUS(OK);
    *     }
*/
#define FCP_RETURN_IF_ERROR(status_)                             \
  do {                                                        \
    if (status_.code() != absl::StatusCode::kOk) {          \
      return (status_);                                      \
    }                                                         \
  } while (false)

/**
 * Macro which allows to check for a StatusOr and return it's status if not OK,
 * otherwise assign the value in the StatusOr to variable or declaration. Usage:
 *
 *     StatusOr<bool> DoSomething() {
 *       FCP_ASSIGN_OR_RETURN(auto value, TryComputeSomething());
 *       if (!value) {
 *         FCP_ASSIGN_OR_RETURN(value, TryComputeSomethingElse());
 *       }
 *       return value;
 *     }
 */
#define FCP_ASSIGN_OR_RETURN(lhs, expr) \
  _FCP_ASSIGN_OR_RETURN_1(              \
      _FCP_ASSIGN_OR_RETURN_CONCAT(statusor_for_aor, __LINE__), lhs, expr)

#define _FCP_ASSIGN_OR_RETURN_1(statusor, lhs, expr) \
  auto statusor = (expr);                            \
  if (!statusor.ok()) {                              \
    return statusor.status();                        \
  }                                                  \
  lhs = std::move(statusor).value()

// See https://goo.gl/x3iba2 for the reason of this construction.
#define _FCP_ASSIGN_OR_RETURN_CONCAT(x, y) \
  _FCP_ASSIGN_OR_RETURN_CONCAT_INNER(x, y)
#define _FCP_ASSIGN_OR_RETURN_CONCAT_INNER(x, y) x##y