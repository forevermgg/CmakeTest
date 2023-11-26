#ifndef BASE_PLATFORM_ANDROID_JNI_UTIL_H_
#define BASE_PLATFORM_ANDROID_JNI_UTIL_H_

#include <jni.h>

#include <optional>
#include <vector>

#include "../macros.h"
#include "scoped_java_ref.h"

namespace base {
namespace jni {

void InitJavaVM(JavaVM* vm);

// Returns a JNI environment for the current thread.
// Attaches the thread to JNI if needed.
JNIEnv* AttachCurrentThread();

void DetachFromVM();

std::optional<JNIEnv*> GetJavaEnv();

std::string JavaStringToString(JNIEnv* env, jstring string);

ScopedJavaLocalRef<jstring> StringToJavaString(JNIEnv* env,
                                               const std::string& str);

std::vector<std::string> StringArrayToVector(JNIEnv* env, jobjectArray jargs);

std::vector<std::string> StringListToVector(JNIEnv* env, jobject list);

ScopedJavaLocalRef<jobjectArray> VectorToStringArray(
    JNIEnv* env, const std::vector<std::string>& vector);

ScopedJavaLocalRef<jobjectArray> VectorToBufferArray(
    JNIEnv* env, const std::vector<std::vector<uint8_t>>& vector);

bool HasException(JNIEnv* env);

bool ClearException(JNIEnv* env, bool silent = false);

bool CheckException(JNIEnv* env);
std::string GetJavaExceptionInfo(JNIEnv* env, jthrowable java_throwable);

// Check for JNI exceptions, print them to the log (if any were raised) and
// clear the exception state returning whether an exception was raised.
bool CheckAndClearException(JNIEnv* env);

// Converts a `java.util.List<String>` to a `std::vector<std::string>`.
std::vector<std::string> JavaStringListToStdStringVector(JNIEnv* env,
                                                         jobject list);

// Convert a `jstring` to a `std::string`.
std::string JavaStringToStdString(JNIEnv* env, jobject string_object);

}  // namespace jni
}  // namespace base

#endif  // BASE_PLATFORM_ANDROID_JNI_UTIL_H_
