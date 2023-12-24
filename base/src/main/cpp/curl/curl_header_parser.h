#pragma once
#include <string>

#include "http_client.h"

// A custom parser that is needed to call the first callback after all the
// headers received
class CurlHeaderParser {
 public:
  explicit CurlHeaderParser();
  ~CurlHeaderParser() = default;
  CurlHeaderParser(const CurlHeaderParser&) = delete;
  CurlHeaderParser& operator=(const CurlHeaderParser&) = delete;

  // Parses the next header string
  void ParseHeader(const std::string& header_string);

  // Removes the "Content-Encoding", "Content-Length", and "Content-Length"
  // headers from the response when the curl encoding in use because they
  // reflect in-flight encoded values
  void UseCurlEncoding();
  // Indicates that the parser reached the last header
  ABSL_MUST_USE_RESULT bool IsLastHeader() const;
  ABSL_MUST_USE_RESULT int GetStatusCode() const;
  ABSL_MUST_USE_RESULT HeaderList GetHeaderList() const;

 private:
  // Extracts status codes from HTTP/1.1 and HTTP/2 responses
  bool ParseAsStatus(const std::string& header_string);
  // Parses a header into a key-value pair
  bool ParseAsHeader(const std::string& header_string);
  // Decides whether it is the last header
  bool ParseAsLastLine(const std::string& header_string);

  int status_code_;
  HeaderList header_list_;
  bool is_last_header_line_;
  bool use_curl_encoding_;
};