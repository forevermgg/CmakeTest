#ifndef BASE_THREAD_LOCAL_H_
#define BASE_THREAD_LOCAL_H_

#include <memory>

#include "macros.h"

#include <pthread.h>

namespace base {

#define BASE_THREAD_LOCAL static

namespace internal {

class ThreadLocalPointer {
 public:
  explicit ThreadLocalPointer(void (*destroy)(void*));
  ~ThreadLocalPointer();

  void* get() const;
  void* swap(void* ptr);

 private:
  pthread_key_t key_;

  BASE_DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer);
};

}  // namespace internal

template <typename T>
class ThreadLocalUniquePtr {
 public:
  ThreadLocalUniquePtr() : ptr_(destroy) {}

  T* get() const { return reinterpret_cast<T*>(ptr_.get()); }
  void reset(T* ptr) { destroy(ptr_.swap(ptr)); }

 private:
  static void destroy(void* ptr) { delete reinterpret_cast<T*>(ptr); }

  internal::ThreadLocalPointer ptr_;

  BASE_DISALLOW_COPY_AND_ASSIGN(ThreadLocalUniquePtr);
};


#define BASE_THREAD_LOCAL static thread_local

template <typename T>
class ThreadLocalUniquePtr2 {
 public:
  ThreadLocalUniquePtr2() = default;

  T* get() const { return ptr_.get(); }
  void reset(T* ptr) { ptr_.reset(ptr); }

 private:
  std::unique_ptr<T> ptr_;

  BASE_DISALLOW_COPY_AND_ASSIGN(ThreadLocalUniquePtr2);
};
}  // namespace base

#endif  // BASE_THREAD_LOCAL_H_
