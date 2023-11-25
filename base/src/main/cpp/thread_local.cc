#include "thread_local.h"

#include <cstring>

#include "logging.h"

namespace base {
namespace internal {

ThreadLocalPointer::ThreadLocalPointer(void (*destroy)(void*)) {
  BASE_CHECK(pthread_key_create(&key_, destroy) == 0);
}

ThreadLocalPointer::~ThreadLocalPointer() {
  BASE_CHECK(pthread_key_delete(key_) == 0);
}

void* ThreadLocalPointer::get() const {
  return pthread_getspecific(key_);
}

void* ThreadLocalPointer::swap(void* ptr) {
  void* old_ptr = get();
  int err = pthread_setspecific(key_, ptr);
  if (err) {
    BASE_CHECK(false) << "pthread_setspecific failed (" << err
                     << "): " << strerror(err);
  }
  return old_ptr;
}

}  // namespace internal
}  // namespace base
