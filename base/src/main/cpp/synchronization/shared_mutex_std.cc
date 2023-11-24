#include "shared_mutex_std.h"
#include "shared_mutex.h"

namespace base {

SharedMutex* SharedMutex::Create() {
  return new SharedMutexStd();
}

void SharedMutexStd::Lock() {
  mutex_.lock();
}

void SharedMutexStd::LockShared() {
  mutex_.lock_shared();
}

void SharedMutexStd::Unlock() {
  mutex_.unlock();
}

void SharedMutexStd::UnlockShared() {
  mutex_.unlock_shared();
}

}  // namespace base
