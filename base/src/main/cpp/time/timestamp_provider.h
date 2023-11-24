#ifndef TIME_TIMESTAMP_PROVIDER_H_
#define TIME_TIMESTAMP_PROVIDER_H_

#include <cstdint>

#include "time_point.h"

namespace base {

/// Pluggable provider of monotonic timestamps. Invocations of `Now` must return
/// unique values. Any two consecutive invocations must be ordered.
class TimestampProvider {
 public:
  virtual ~TimestampProvider(){};

  // Returns the number of ticks elapsed by a monotonic clock since epoch.
  virtual base::TimePoint Now() = 0;
};

}  // namespace base

#endif  // TIME_TIMESTAMP_PROVIDER_H_
