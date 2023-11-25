#include <jni.h>
#include "jni/jni_util.h"
#include "log_settings.h"
#include "logging.h"
#include "util/event_loop.hpp"

// This is called by the VM when the shared library is first loaded.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::LogSettings log_settings;
  log_settings.min_log_level = base::kLogInfo;
  base::SetLogSettings(log_settings);
  // Initialize the Java VM.
  base::jni::InitJavaVM(vm);
  JNIEnv* env = base::jni::AttachCurrentThread();
  if (base::util::EventLoop::has_implementation()) {
    BASE_LOG(ERROR) << "has_implementation";
  } else {
    BASE_LOG(ERROR) << "not has_implementation";
  }
  base::util::EventLoop::main().run_until([&] {
    BASE_LOG(ERROR) << "base::util::EventLoop::main().run_until";
    return true;
  });
  BASE_LOG(ERROR) << "JNI_OnLoad";
  return JNI_VERSION_1_4;
}
