#ifndef FOREVER_JNI_UTIL_JAVA_CLASS_HPP
#define FOREVER_JNI_UTIL_JAVA_CLASS_HPP

#include <jni.h>

#include "java_global_ref_by_move.hpp"

namespace FOREVER {
namespace JNI_UTIL {

// To find the jclass and manage the lifecycle for the jclass's global ref.
class JavaClass {
 public:
  JavaClass();
  // when free_on_unload is true, the jclass's global ref will be released when
  // JNI_OnUnload called. This is useful when the JavaClass instance is static.
  // Otherwise the jclass's global ref will be released when this object is
  // deleted.
  JavaClass(JNIEnv* env, const char* class_name, bool free_on_unload = true);
  ~JavaClass() {}

  JavaClass(JavaClass&&);

  inline jclass get() noexcept { return m_class; }

  inline operator jclass() const noexcept { return m_class; }

  inline operator bool() const noexcept { return m_class != nullptr; }

  // Not implemented for now.
  JavaClass(JavaClass&) = delete;
  JavaClass& operator=(JavaClass&&) = delete;

 private:
  JavaGlobalRefByMove m_ref_owner;
  jclass m_class;
  static JavaGlobalRefByMove get_jclass(JNIEnv* env, const char* class_name);
};

}  // namespace JNI_UTIL
}  // namespace FOREVER

#endif
