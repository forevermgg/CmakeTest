#include "java_class.hpp"

#include "jni_utils.hpp"

using namespace FOREVER::JNI_UTIL;

JavaClass::JavaClass() : m_ref_owner(), m_class(nullptr) {}

JavaClass::JavaClass(JNIEnv* env, const char* class_name, bool free_on_unload)
    : m_ref_owner(get_jclass(env, class_name)),
      m_class(reinterpret_cast<jclass>(m_ref_owner.get())) {
  if (free_on_unload) {
    // Move the ownership of global ref to JNIUtils which will be released when
    // JNI_OnUnload.
    JniUtils::keep_global_ref(m_ref_owner);
  }
}

JavaClass::JavaClass(JavaClass&& rhs)
    : m_ref_owner(std::move(rhs.m_ref_owner)), m_class(rhs.m_class) {
  rhs.m_class = nullptr;
}

JavaGlobalRefByMove JavaClass::get_jclass(JNIEnv* env, const char* class_name) {
  jclass cls = env->FindClass(class_name);
  // REALM_ASSERT_RELEASE_EX(cls, class_name);

  JavaGlobalRefByMove cls_ref(env, cls, true);
  return cls_ref;
}
