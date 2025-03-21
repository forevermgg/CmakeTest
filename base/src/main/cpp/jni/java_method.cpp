#include "java_method.hpp"

#include "java_class.hpp"

using namespace FOREVER::JNI_UTIL;

JavaMethod::JavaMethod(JNIEnv* env, JavaClass const& cls,
                       const char* method_name, const char* signature,
                       bool static_method) {
  if (static_method) {
    m_method_id = env->GetStaticMethodID(cls, method_name, signature);
  } else {
    m_method_id = env->GetMethodID(cls, method_name, signature);
  }

  // FOREVER_ASSERT_RELEASE_EX(m_method_id != nullptr, method_name, signature);
}

JavaMethod::JavaMethod(JNIEnv* env, jobject const& obj, const char* method_name,
                       const char* signature) {
  jclass cls = env->GetObjectClass(obj);
  m_method_id = env->GetMethodID(cls, method_name, signature);
  // FOREVER_ASSERT_RELEASE_EX(m_method_id != nullptr, method_name, signature);
}
