#include "count_down_latch.h"

namespace base {

CountDownLatch::CountDownLatch(size_t count) : count_(count) {
  if (count_ == 0) {
    waitable_event_.Signal();
  }
}

CountDownLatch::~CountDownLatch() = default;

void CountDownLatch::Wait() {
  waitable_event_.Wait();
}

void CountDownLatch::CountDown() {
  if (--count_ == 0) {
    waitable_event_.Signal();
  }
}

}  // namespace base
