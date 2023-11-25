#ifndef FOREVER_JNI_UTIL_JNI_UTILS_HPP
#define FOREVER_JNI_UTIL_JNI_UTILS_HPP

#include <jni.h>

#include <map>
#include <vector>

#include "java_global_ref_by_copy.hpp"
#include "java_global_ref_by_move.hpp"

namespace FOREVER {
namespace JNI_UTIL {

// Util functions for JNI.
class JniUtils {
 public:
  ~JniUtils() {}

  // Call this only once in JNI_OnLoad.
  static void initialize(JavaVM* vm, jint vm_version) noexcept;
  // Call this in JNI_OnUnload.
  static void release();
  // When attach_if_needed is false, returns the JNIEnv if there is one attached
  // to this thread. Assert if there is none. When attach_if_needed is true, try
  // to attach and return a JNIEnv if necessary.
  static JNIEnv* get_env(bool attach_if_needed = false);
  // Detach the current thread from the JVM. Only required for C++ threads that
  // where attached in the first place. Failing to do so is a resource leak.
  static void detach_current_thread();
  // Keep the given global reference until JNI_OnUnload is called.
  static void keep_global_ref(JavaGlobalRefByMove& ref);
  static std::string JavaStringToString(JNIEnv* env, jstring string);
  static jobject to_hash_map(JNIEnv* env,
                             std::map<std::string, std::string> map);

 private:
  JniUtils(JavaVM* vm, jint vm_version) noexcept
      : m_vm(vm), m_vm_version(vm_version) {}

  JavaVM* m_vm;
  jint m_vm_version;
  std::vector<JavaGlobalRefByMove> m_global_refs;
};

}  // namespace JNI_UTIL
}  // namespace FOREVER

#endif  // FOREVER_JNI_UTIL_JNI_UTILS_HPP
