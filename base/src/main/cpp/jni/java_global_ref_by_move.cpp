#include "java_global_ref_by_move.hpp"

#include <memory>

#include "jni_utils.hpp"

using namespace FOREVER::JNI_UTIL;

JavaGlobalRefByMove::~JavaGlobalRefByMove() {
  if (m_ref) {
    JniUtils::get_env()->DeleteGlobalRef(m_ref);
  }
}

JavaGlobalRefByMove& JavaGlobalRefByMove::operator=(JavaGlobalRefByMove&& rhs) {
  this->~JavaGlobalRefByMove();
  new (this) JavaGlobalRefByMove(std::move(rhs));
  return *this;
}

JavaGlobalRefByMove::JavaGlobalRefByMove(JavaGlobalRefByMove& rhs)
    : m_ref(rhs.m_ref
                ? JNI_UTIL::JniUtils::get_env(true)->NewGlobalRef(rhs.m_ref)
                : nullptr) {}
