#include "string_conversion.h"

#include <codecvt>
#include <locale>
#include <sstream>
#include <string>

namespace base {

using Utf16StringConverter =
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>;

std::string Join(const std::vector<std::string>& vec, const char* delim) {
  std::stringstream res;
  for (size_t i = 0; i < vec.size(); ++i) {
    res << vec[i];
    if (i < vec.size() - 1) {
      res << delim;
    }
  }
  return res.str();
}

std::string Utf16ToUtf8(const std::u16string_view string) {
  Utf16StringConverter converter;
  return converter.to_bytes(string.data());
}

std::u16string Utf8ToUtf16(const std::string_view string) {
  Utf16StringConverter converter;
  return converter.from_bytes(string.data());
}

}  // namespace base
