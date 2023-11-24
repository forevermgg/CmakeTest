#include "time_point.h"

#include <atomic>
#include <chrono>

namespace base {

namespace {
std::atomic<TimePoint::ClockSource> gSteadyClockSource;
}

template <typename Clock, typename Duration>
static int64_t NanosSinceEpoch(
    std::chrono::time_point<Clock, Duration> time_point) {
  const auto elapsed = time_point.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
}

void TimePoint::SetClockSource(ClockSource source) {
  gSteadyClockSource = source;
}

TimePoint TimePoint::Now() {
  if (gSteadyClockSource) {
    return gSteadyClockSource.load()();
  }
  const int64_t nanos = NanosSinceEpoch(std::chrono::steady_clock::now());
  return TimePoint(nanos);
}

TimePoint TimePoint::CurrentWallTime() {
  const int64_t nanos = NanosSinceEpoch(std::chrono::system_clock::now());
  return TimePoint(nanos);
}
}  // namespace fml
