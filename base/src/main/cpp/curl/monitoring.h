
#define FCP_PREDICT_FALSE(x) ABSL_PREDICT_FALSE(x)
#define FCP_PREDICT_TRUE(x) ABSL_PREDICT_TRUE(x)

#define _FCP_LOG_WARNING ABSL_LOG(WARNING) << "test: "
#define _FCP_LOG_ERROR ABSL_LOG(ERROR) << "test: "
#define _FCP_LOG_FATAL ABSL_LOG(FATAL) << "test: "
#define _FCP_LOG_IF_WARNING(condition) \
  ABSL_LOG_IF(WARNING, condition) << "test: "
#define _FCP_LOG_IF_ERROR(condition) ABSL_LOG_IF(ERROR, condition) << "test: "
#define _FCP_LOG_IF_FATAL(condition) ABSL_LOG_IF(FATAL, condition) << "test: "

#define FCP_LOG_IF(severity, condition) \
  !(condition) ? (void)0                \
               : _FCP_LOG_##severity
/**
 * Check that the condition holds, otherwise die. Any additional messages can
 * be streamed into the invocation. Example:
 *
 *     FCP_CHECK(condition) << "stuff went wrong";
 */
#define FCP_CHECK(condition)                         \
  FCP_LOG_IF(FATAL, FCP_PREDICT_FALSE(!(condition))) \
      << ("Check failed: " #condition ". ")