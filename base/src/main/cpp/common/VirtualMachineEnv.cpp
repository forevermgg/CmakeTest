#include "VirtualMachineEnv.h"

#include "debug.h"
JavaVM *VirtualMachineEnv::sVirtualMachine = nullptr;

// This is called when the library is loaded. We need this to get a reference to
// the global VM
UTILS_NOINLINE
jint VirtualMachineEnv::JNI_OnLoad(JavaVM *vm) noexcept {
  JNIEnv *env = nullptr;
  if (UTILS_UNLIKELY(vm->GetEnv(reinterpret_cast<void **>(&env),
                                JNI_VERSION_1_6) != JNI_OK)) {
    // this should not happen
    return -1;
  }
  sVirtualMachine = vm;
  return JNI_VERSION_1_6;
}

UTILS_NOINLINE
void VirtualMachineEnv::handleException(JNIEnv *const env) noexcept {
  if (UTILS_UNLIKELY(env->ExceptionCheck())) {
    env->ExceptionDescribe();
    env->ExceptionClear();
  }
}

UTILS_NOINLINE
JNIEnv *VirtualMachineEnv::getEnvironmentSlow() noexcept {
#if defined(__ANDROID__)
  mVirtualMachine->AttachCurrentThread(&mJniEnv, nullptr);
#else
  mVirtualMachine->AttachCurrentThread(reinterpret_cast<void **>(&mJniEnv),
                                       nullptr);
#endif
  __android_log_print(ANDROID_LOG_ERROR, "AttachCurrentThread",
                      "VirtualMachineEnv AttachCurrentThread");
  assert_invariant(mJniEnv);
  return mJniEnv;
}
