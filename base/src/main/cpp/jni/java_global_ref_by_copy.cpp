#include "java_global_ref_by_copy.hpp"

#include <memory>

#include "jni_utils.hpp"

using namespace FOREVER::JNI_UTIL;

JavaGlobalRefByCopy::JavaGlobalRefByCopy() : m_ref(nullptr) {}

JavaGlobalRefByCopy::JavaGlobalRefByCopy(JNIEnv *env, jobject obj)
    : m_ref(obj ? env->NewGlobalRef(obj) : nullptr) {}

JavaGlobalRefByCopy::JavaGlobalRefByCopy(const JavaGlobalRefByCopy &rhs)
    : m_ref(rhs.m_ref ? JniUtils::get_env(true)->NewGlobalRef(rhs.m_ref)
                      : nullptr) {}

JavaGlobalRefByCopy::~JavaGlobalRefByCopy() {
  if (m_ref) {
    JniUtils::get_env()->DeleteGlobalRef(m_ref);
  }
}

jobject JavaGlobalRefByCopy::get() const noexcept { return m_ref; }
