#include "chrono_timestamp_provider.h"

#include <chrono>

namespace base {

ChronoTimestampProvider::ChronoTimestampProvider() = default;

ChronoTimestampProvider::~ChronoTimestampProvider() = default;

base::TimePoint ChronoTimestampProvider::Now() {
  const auto chrono_time_point = std::chrono::steady_clock::now();
  const auto ticks_since_epoch = chrono_time_point.time_since_epoch().count();
  return base::TimePoint::FromTicks(ticks_since_epoch);
}

base::TimePoint ChronoTicksSinceEpoch() {
  return ChronoTimestampProvider::Instance().Now();
}

}  // namespace base
