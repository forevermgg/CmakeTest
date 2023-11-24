#ifndef SYNCHRONIZATION_ATOMIC_OBJECT_H_
#define SYNCHRONIZATION_ATOMIC_OBJECT_H_

#include <mutex>

namespace base {

// A wrapper for an object instance that can be read or written atomically.
template <typename T>
class AtomicObject {
 public:
  AtomicObject() = default;
  explicit AtomicObject(T object) : object_(object) {}

  T Load() const {
    std::scoped_lock lock(mutex_);
    return object_;
  }

  void Store(const T& object) {
    std::scoped_lock lock(mutex_);
    object_ = object;
  }

 private:
  mutable std::mutex mutex_;
  T object_;
};

}  // namespace base

#endif  // SYNCHRONIZATION_ATOMIC_OBJECT_H_
