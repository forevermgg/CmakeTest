#ifndef STD_STRIN_GEXTRACTOR_H
#define STD_STRIN_GEXTRACTOR_H

#include <stdint.h>

#include <string>

// Based on StringExtractor, with the added limitation that this file should not
// take a dependency on LLVM, as it is used from debugserver.
class StdStringExtractor {
 public:
  enum { BigEndian = 0, LittleEndian = 1 };
  // Constructors and Destructors
  StdStringExtractor();
  StdStringExtractor(const char *packet_cstr);
  virtual ~StdStringExtractor();

  // Returns true if the file position is still valid for the data
  // contained in this string extractor object.
  bool IsGood() const { return m_index != UINT64_MAX; }

  uint64_t GetFilePos() const { return m_index; }

  void SetFilePos(uint32_t idx) { m_index = idx; }

  void Clear() {
    m_packet.clear();
    m_index = 0;
  }

  void SkipSpaces();

  const std::string &GetStringRef() const { return m_packet; }

  bool Empty() { return m_packet.empty(); }

  size_t GetBytesLeft() {
    if (m_index < m_packet.size()) return m_packet.size() - m_index;
    return 0;
  }

  char GetChar(char fail_value = '\0');

  char PeekChar(char fail_value = '\0') {
    const char *cstr = Peek();
    if (cstr) return cstr[0];
    return fail_value;
  }

  int DecodeHexU8();

  uint8_t GetHexU8(uint8_t fail_value = 0, bool set_eof_on_fail = true);

  bool GetHexU8Ex(uint8_t &ch, bool set_eof_on_fail = true);

  bool GetNameColonValue(std::string &name, std::string &value);

  int32_t GetS32(int32_t fail_value, int base = 0);

  uint32_t GetU32(uint32_t fail_value, int base = 0);

  int64_t GetS64(int64_t fail_value, int base = 0);

  uint64_t GetU64(uint64_t fail_value, int base = 0);

  uint32_t GetHexMaxU32(bool little_endian, uint32_t fail_value);

  uint64_t GetHexMaxU64(bool little_endian, uint64_t fail_value);

  size_t GetHexBytes(void *dst, size_t dst_len, uint8_t fail_fill_value);

  size_t GetHexBytesAvail(void *dst, size_t dst_len);

  size_t GetHexByteString(std::string &str);

  size_t GetHexByteStringFixedLength(std::string &str, uint32_t nibble_length);

  size_t GetHexByteStringTerminatedBy(std::string &str, char terminator);

  const char *Peek() {
    if (m_index < m_packet.size()) return m_packet.c_str() + m_index;
    return nullptr;
  }

 protected:
  // For StdStringExtractor only
  std::string m_packet;  // The string in which to extract data.
  uint64_t m_index;      // When extracting data from a packet, this index
                         // will march along as things get extracted. If set
                         // to UINT64_MAX the end of the packet data was
                         // reached when decoding information
};

#endif  // STD_STRIN_GEXTRACTOR_H
