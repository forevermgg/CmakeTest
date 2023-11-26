#ifndef FOREVER_EMBEDDED_FILE_H_
#define FOREVER_EMBEDDED_FILE_H_

#include <stddef.h>

#include <vector>

namespace FOREVER {
namespace INTERNAL {

// File embedded in the binary.
struct EmbeddedFile {
  EmbeddedFile() : name(nullptr), data(nullptr), size(0) {}
  EmbeddedFile(const char* _name, const unsigned char* _data, size_t _size)
      : name(_name), data(_data), size(_size) {}

  const char* name;
  const unsigned char* data;
  size_t size;

  // Create a vector with a single EmbeddedFile structure.
  static std::vector<EmbeddedFile> ToVector(const char* name,
                                            const unsigned char* data,
                                            size_t size) {
    std::vector<EmbeddedFile> vector;
    vector.push_back(EmbeddedFile(name, data, size));
    return vector;
  }
};

}  // namespace INTERNAL
}  // namespace FOREVER

#endif  // FOREVER_EMBEDDED_FILE_H_
