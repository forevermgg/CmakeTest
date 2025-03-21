#include "curl_http_response.h"

#include <utility>

CurlHttpResponse::CurlHttpResponse(int status_code, HeaderList header_list)
    : status_code_(status_code), header_list_(std::move(header_list)) {}

int CurlHttpResponse::code() const { return status_code_; }

const HeaderList& CurlHttpResponse::headers() const { return header_list_; }
