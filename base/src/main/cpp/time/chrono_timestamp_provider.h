#ifndef TIME_CHRONO_TIMESTAMP_PROVIDER_H_
#define TIME_CHRONO_TIMESTAMP_PROVIDER_H_

#include "timestamp_provider.h"
#include "time_point.h"
#include "../macros.h"

namespace base {

/// TimestampProvider implementation that is backed by std::chrono::steady_clock
/// meant to be used only in tests for `fml`. Other components needing the
/// current time ticks since epoch should instantiate their own time stamp
/// provider backed by Dart clock.
class ChronoTimestampProvider : TimestampProvider {
 public:
  static ChronoTimestampProvider& Instance() {
    static ChronoTimestampProvider instance;
    return instance;
  }

  ~ChronoTimestampProvider() override;

  base::TimePoint Now() override;

 private:
  ChronoTimestampProvider();

  BASE_DISALLOW_COPY_AND_ASSIGN(ChronoTimestampProvider);
};

base::TimePoint ChronoTicksSinceEpoch();

}  // namespace base

#endif  // TIME_CHRONO_TIMESTAMP_PROVIDER_H_
