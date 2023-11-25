#ifndef LOGGING_H_
#define LOGGING_H_

#include <sstream>

#include "log_level.h"
#include "macros.h"

namespace base {

namespace testing {
struct LogCapture {
  LogCapture();
  ~LogCapture();

  std::string str() const;

 private:
  std::ostringstream stream_;
};
}  // namespace testing

class LogMessageVoidify {
 public:
  void operator&(std::ostream&) {}
};

class LogMessage {
 public:
  LogMessage(LogSeverity severity,
             const char* file,
             int line,
             const char* condition);
  ~LogMessage();

  std::ostream& stream() { return stream_; }

  static void CaptureNextLog(std::ostringstream* stream);

 private:
  // This is a raw pointer so that we avoid having a non-trivially-destructible
  // static. It is only ever for use in unit tests.
  static thread_local std::ostringstream* capture_next_log_stream_;
  std::ostringstream stream_;
  const LogSeverity severity_;
  const char* file_;
  const int line_;

  BASE_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

// Gets the BASE_VLOG default verbosity level.
int GetVlogVerbosity();

// Returns true if |severity| is at or above the current minimum log level.
// kLogFatal and above is always true.
bool ShouldCreateLogMessage(LogSeverity severity);

[[noreturn]] void KillProcess();

}  // namespace base

#define BASE_LOG_STREAM(severity) \
  ::base::LogMessage(::base::LOG_##severity, __FILE__, __LINE__, nullptr).stream()

#define BASE_LAZY_STREAM(stream, condition) \
  !(condition) ? (void)0 : ::base::LogMessageVoidify() & (stream)

#define BASE_EAT_STREAM_PARAMETERS(ignored) \
  true || (ignored)                        \
      ? (void)0                            \
      : ::base::LogMessageVoidify() &       \
            ::base::LogMessage(::base::kLogFatal, 0, 0, nullptr).stream()

#define BASE_LOG_IS_ON(severity) \
  (::base::ShouldCreateLogMessage(::base::LOG_##severity))

#define BASE_LOG(severity) \
  BASE_LAZY_STREAM(BASE_LOG_STREAM(severity), BASE_LOG_IS_ON(severity))

#define BASE_CHECK(condition)                                              \
  BASE_LAZY_STREAM(                                                        \
      ::base::LogMessage(::base::kLogFatal, __FILE__, __LINE__, #condition) \
          .stream(),                                                      \
      !(condition))

#define BASE_VLOG_IS_ON(verbose_level) \
  ((verbose_level) <= ::base::GetVlogVerbosity())

// The VLOG macros log with negative verbosities.
#define BASE_VLOG_STREAM(verbose_level) \
  ::base::LogMessage(-verbose_level, __FILE__, __LINE__, nullptr).stream()

#define BASE_VLOG(verbose_level) \
  BASE_LAZY_STREAM(BASE_VLOG_STREAM(verbose_level), BASE_VLOG_IS_ON(verbose_level))

#ifndef NDEBUG
#define BASE_DLOG(severity) BASE_LOG(severity)
#define BASE_DCHECK(condition) BASE_CHECK(condition)
#else
#define BASE_DLOG(severity) BASE_EAT_STREAM_PARAMETERS(true)
#define BASE_DCHECK(condition) BASE_EAT_STREAM_PARAMETERS(condition)
#endif

#if NDEBUG
#define BASE_ASSERT(condition) BASE_CHECK(condition)
#else
#define BASE_ASSERT(condition) static_cast<void>(sizeof bool(condition))
#endif

#define BASE_UNREACHABLE()                          \
  {                                                \
    BASE_LOG(ERROR) << "Reached unreachable code."; \
    ::base::KillProcess();                          \
  }

#endif  // BASE_LOGGING_H_
