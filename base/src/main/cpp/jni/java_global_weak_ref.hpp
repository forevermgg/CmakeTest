#ifndef FOREVER_JNI_UTIL_JAVA_GLOBAL_WEAK_REF_HPP
#define FOREVER_JNI_UTIL_JAVA_GLOBAL_WEAK_REF_HPP

#include <jni.h>

#include <functional>

#include "jni_utils.hpp"

namespace FOREVER {
namespace JNI_UTIL {

// RAII wrapper for weak global ref.
class JavaGlobalWeakRef {
 public:
  JavaGlobalWeakRef();
  JavaGlobalWeakRef(JNIEnv*, jobject);
  ~JavaGlobalWeakRef();

  JavaGlobalWeakRef(JavaGlobalWeakRef&&);
  JavaGlobalWeakRef& operator=(JavaGlobalWeakRef&&);

  JavaGlobalWeakRef(const JavaGlobalWeakRef&);
  JavaGlobalWeakRef& operator=(const JavaGlobalWeakRef&);

  inline operator bool() const noexcept { return m_weak != nullptr; }

  JavaGlobalRefByMove global_ref(JNIEnv* env = nullptr) const;

  using Callback = void(JNIEnv* env, jobject obj);

  // Acquire a local ref and run the callback with it if the weak ref is valid.
  // The local ref will be deleted after callback finished. Return false if the
  // weak ref is not valid anymore.
  bool call_with_local_ref(JNIEnv* env, std::function<Callback> callback) const;
  // Try to get an JNIEnv for current thread then run the callback.
  bool call_with_local_ref(std::function<Callback> callback) const;

 private:
  jweak m_weak;
};

}  // namespace JNI_UTIL
}  // namespace FOREVER

#endif  // FOREVER_JNI_UTIL_JAVA_GLOBAL_WEAK_REF_HPP
