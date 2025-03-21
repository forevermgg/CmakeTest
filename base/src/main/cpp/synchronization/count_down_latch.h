#ifndef SYNCHRONIZATION_COUNT_DOWN_LATCH_H_
#define SYNCHRONIZATION_COUNT_DOWN_LATCH_H_

#include <atomic>

#include "../macros.h"
#include "waitable_event.h"

namespace base {

class CountDownLatch {
 public:
  explicit CountDownLatch(size_t count);

  ~CountDownLatch();

  void Wait();

  void CountDown();

 private:
  std::atomic_size_t count_;
  ManualResetWaitableEvent waitable_event_;

  BASE_DISALLOW_COPY_AND_ASSIGN(CountDownLatch);
};

}  // namespace base

#endif  // SYNCHRONIZATION_COUNT_DOWN_LATCH_H_
