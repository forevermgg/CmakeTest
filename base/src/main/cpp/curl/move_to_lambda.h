#pragma once
#include <utility>

/**
 * Copyable wrapper for a move-only value. See MoveToLambda.
 * The value is accessible with the * and -> operators.
 *
 * You must be careful to avoid accidnetal copies of this type. Copies are
 * destructive (by design), so accidental copies might lead to using a
 */
template <typename T>
class MoveToLambdaWrapper {
 public:
  explicit MoveToLambdaWrapper(T t) : value_(std::move(t)) {}

  // The copy and move constructors are intentionally non-const.

  MoveToLambdaWrapper(MoveToLambdaWrapper const& other)
      : value_(std::move(other.value_)) {}

  MoveToLambdaWrapper& operator=(MoveToLambdaWrapper const& other) {
    value_ = std::move(other.value_);
    return *this;
  }

  // We respect const-ness of the wrapper when dereferencing, so that 'mutable'
  // is required on the lambda depending on usage of the value; changes
  // to a captured value persist across calls to the lambda, which is rarely
  // desired.

  T const& operator*() const & {
    return value_;
  }

  T const* operator->() const & {
    return &value_;
  }

  T& operator*() & {
    return value_;
  }

  T* operator->() & {
    return &value_;
  }

 private:
  mutable T value_;
};

/**
 * Allows capturing a value into a lambda 'by move', before C++14. This is
 * implemented by a copyable wrapper, which actually moves its value.
 *
 *     auto moving = MoveToLambda(value);
 *     DoSometing([moving]{ V const& v = *moving; ... });
 */
template <typename T>
MoveToLambdaWrapper<std::remove_reference_t<T>> MoveToLambda(T&& value) {
  static_assert(
      std::is_rvalue_reference<T&&>::value,
      "Expected an rvalue: If the value is copied anyway (to this function), "
      "you might as well put it in the lambda-capture list directly.");
  return MoveToLambdaWrapper<std::remove_reference_t<T>>(
      std::forward<T>(value));

}

