#ifndef SYNCHRONIZATION_SHARED_MUTEX_STD_H_
#define SYNCHRONIZATION_SHARED_MUTEX_STD_H_

#include <shared_mutex>
#include "shared_mutex.h"

namespace base {

class SharedMutexStd : public SharedMutex {
 public:
  virtual void Lock();
  virtual void LockShared();
  virtual void Unlock();
  virtual void UnlockShared();

 private:
  friend SharedMutex* SharedMutex::Create();
  SharedMutexStd() = default;

  std::shared_timed_mutex mutex_;
};

}  // namespace fml

#endif  // SYNCHRONIZATION_SHARED_MUTEX_STD_H_
