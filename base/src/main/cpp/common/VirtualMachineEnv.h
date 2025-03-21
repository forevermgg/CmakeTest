#include <android/log.h>
#include <jni.h>
#include "debug.h"

class VirtualMachineEnv {
 public:
  static jint JNI_OnLoad(JavaVM *vm) noexcept;

  static VirtualMachineEnv &get() noexcept {
    // declaring this thread local, will ensure it's destroyed with the calling
    // thread
    static thread_local VirtualMachineEnv instance;
    return instance;
  }

  static JNIEnv *getThreadEnvironment() noexcept {
    JNIEnv *env;
    assert_invariant(sVirtualMachine);
    if (sVirtualMachine->GetEnv(reinterpret_cast<void **>(&env),
                                JNI_VERSION_1_6) != JNI_OK) {
      return nullptr;  // this should not happen
    }
    return env;
  }

  inline JNIEnv *getEnvironment() noexcept {
    assert_invariant(mVirtualMachine);
    JNIEnv *env = mJniEnv;
    if (UTILS_UNLIKELY(!env)) {
      return getEnvironmentSlow();
    }
    return env;
  }

  static void handleException(JNIEnv *env) noexcept;

 private:
  VirtualMachineEnv() noexcept : mVirtualMachine(sVirtualMachine) {
    // We're not initializing the JVM here -- but we could -- because most of
    // the time we don't need the jvm. Instead we do the initialization on first
    // use. This means we could get a nasty slow down the very first time, but
    // we'll live with it for now.
  }

  ~VirtualMachineEnv() {
    if (mVirtualMachine) {
      __android_log_print(ANDROID_LOG_ERROR, "DetachCurrentThread",
                          "VirtualMachineEnv DetachCurrentThread");
      mVirtualMachine->DetachCurrentThread();
    }
  }

  JNIEnv *getEnvironmentSlow() noexcept;
  static JavaVM *sVirtualMachine;
  JNIEnv *mJniEnv = nullptr;
  JavaVM *mVirtualMachine = nullptr;
};