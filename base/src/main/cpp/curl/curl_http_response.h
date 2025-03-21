
#include <utility>

#include "http_client.h"

// A simple http response. This class is thread-safe.
class CurlHttpResponse : public HttpResponse {
 public:
  CurlHttpResponse(int status_code, HeaderList header_list);
  ~CurlHttpResponse() override = default;

  // HttpResponse:
  ABSL_MUST_USE_RESULT int code() const override;
  ABSL_MUST_USE_RESULT const HeaderList& headers() const override;

 private:
  const int status_code_;
  const HeaderList header_list_;
};
