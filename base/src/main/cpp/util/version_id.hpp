#ifndef BASE_VERSION_ID_HPP
#define BASE_VERSION_ID_HPP

#include <cstdint>
#include <limits>
#include <ostream>

namespace base {

struct VersionID {
  using version_type = uint_fast64_t;
  version_type version = std::numeric_limits<version_type>::max();
  uint_fast32_t index = 0;

  constexpr VersionID() = default;
  constexpr VersionID(version_type initial_version,
                      uint_fast32_t initial_index) noexcept {
    version = initial_version;
    index = initial_index;
  }

  constexpr bool operator==(const VersionID& other) const noexcept {
    return version == other.version;
  }
  constexpr bool operator!=(const VersionID& other) const noexcept {
    return version != other.version;
  }
  constexpr bool operator<(const VersionID& other) const noexcept {
    return version < other.version;
  }
  constexpr bool operator<=(const VersionID& other) const noexcept {
    return version <= other.version;
  }
  constexpr bool operator>(const VersionID& other) const noexcept {
    return version > other.version;
  }
  constexpr bool operator>=(const VersionID& other) const noexcept {
    return version >= other.version;
  }
};

inline std::ostream& operator<<(std::ostream& os, VersionID id) {
  os << "VersionID(" << id.version << ", " << id.index << ")";
  return os;
}

}  // namespace base

#endif  // BASE_VERSION_ID_HPP
