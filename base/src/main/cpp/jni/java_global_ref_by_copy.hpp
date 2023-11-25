#ifndef FOREVER_JNI_UTIL_JAVA_GLOBAL_REF_COPY_HPP
#define FOREVER_JNI_UTIL_JAVA_GLOBAL_REF_COPY_HPP

#include <jni.h>

namespace FOREVER {
namespace JNI_UTIL {

// Manages the lifecycle of jobject's global ref via copy constructors
//
// It prevents leaking global references by automatically referencing and
// unreferencing Java objects any time the instance is copied or destroyed. Its
// principal use is on data structures that don't support moving operations such
// as in std::function lambdas.
//
// Note that there is another flavor available: JavaGlobalRefByMove.
//
// JavaGlobalRefByCopy: multiple references will exist to the Java object, one
// on each instance. JavaGlobalRefByMove: only one reference will only be
// available at last moved instance.

class JavaGlobalRefByCopy {
 public:
  JavaGlobalRefByCopy();

  JavaGlobalRefByCopy(JNIEnv *env, jobject obj);

  JavaGlobalRefByCopy(const JavaGlobalRefByCopy &rhs);

  JavaGlobalRefByCopy(JavaGlobalRefByCopy &&rhs) = delete;

  ~JavaGlobalRefByCopy();

  jobject get() const noexcept;

 private:
  jobject m_ref;
};
}  // namespace JNI_UTIL
}  // namespace FOREVER

#endif  // FOREVER_JNI_UTIL_JAVA_GLOBAL_REF_COPY_HPP
