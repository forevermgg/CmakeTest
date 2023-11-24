#ifndef LOG_LEVEL_H_
#define LOG_LEVEL_H_

namespace base {

// Default log levels. Negative values can be used for verbose log levels.
typedef int LogSeverity;

constexpr LogSeverity kLogInfo = 0;
constexpr LogSeverity kLogWarning = 1;
constexpr LogSeverity kLogError = 2;
constexpr LogSeverity kLogFatal = 3;
constexpr LogSeverity kLogNumSeverities = 4;

// DEPRECATED: Use |kLogInfo|.
// Ignoring Clang Tidy because this is used in a very common substitution macro.
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr LogSeverity LOG_INFO = kLogInfo;

// DEPRECATED: Use |kLogWarning|.
// Ignoring Clang Tidy because this is used in a very common substitution macro.
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr LogSeverity LOG_WARNING = kLogWarning;

// DEPRECATED: Use |kLogError|.
// Ignoring Clang Tidy because this is used in a very common substitution macro.
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr LogSeverity LOG_ERROR = kLogError;

// DEPRECATED: Use |kLogFatal|.
// Ignoring Clang Tidy because this is used in a very common substitution macro.
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr LogSeverity LOG_FATAL = kLogFatal;

// kLogDFatal is kLogFatal in debug mode, kLogError in normal mode
#ifdef NDEBUG
const LogSeverity kLogDFatal = kLogError;
#else
const LogSeverity kLogDFatal = kLogFatal;
#endif

// DEPRECATED: Use |kLogDFatal|.
// Ignoring Clang Tidy because this is used in a very common substitution macro.
// NOLINTNEXTLINE(readability-identifier-naming)
const LogSeverity LOG_DFATAL = kLogDFatal;

}  // namespace base

#endif  // LOG_LEVEL_H_
