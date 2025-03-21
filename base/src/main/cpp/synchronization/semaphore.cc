#include "semaphore.h"

#include <semaphore.h>
#include "../eintr_wrapper.h"
#include "../logging.h"

namespace base {

class PlatformSemaphore {
 public:
  explicit PlatformSemaphore(uint32_t count)
      : valid_(::sem_init(&sem_, 0 /* not shared */, count) == 0) {}

  ~PlatformSemaphore() {
    if (valid_) {
      int result = ::sem_destroy(&sem_);
      (void)result;
      // Can only be EINVAL which should not be possible since we checked for
      // validity.
      BASE_DCHECK(result == 0);
    }
  }

  bool IsValid() const { return valid_; }

  bool Wait() {
    if (!valid_) {
      return false;
    }

    return BASE_HANDLE_EINTR(::sem_wait(&sem_)) == 0;
  }

  bool TryWait() {
    if (!valid_) {
      return false;
    }

    return BASE_HANDLE_EINTR(::sem_trywait(&sem_)) == 0;
  }

  void Signal() {
    if (!valid_) {
      return;
    }

    ::sem_post(&sem_);

    return;
  }

 private:
  bool valid_;
  sem_t sem_;

  BASE_DISALLOW_COPY_AND_ASSIGN(PlatformSemaphore);
};

}  // namespace base

namespace base {

Semaphore::Semaphore(uint32_t count) : impl_(new PlatformSemaphore(count)) {}

Semaphore::~Semaphore() = default;

bool Semaphore::IsValid() const {
  return impl_->IsValid();
}

bool Semaphore::Wait() {
  return impl_->Wait();
}

bool Semaphore::TryWait() {
  return impl_->TryWait();
}

void Semaphore::Signal() {
  return impl_->Signal();
}

}  // namespace base
