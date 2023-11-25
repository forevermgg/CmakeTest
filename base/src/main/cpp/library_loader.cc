#include <jni.h>
#include "jni/jni_util.h"
#include "log_settings.h"
#include "logging.h"

// This is called by the VM when the shared library is first loaded.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::LogSettings log_settings;
  log_settings.min_log_level = base::kLogInfo;
  base::SetLogSettings(log_settings);
  // Initialize the Java VM.
  base::jni::InitJavaVM(vm);
  JNIEnv* env = base::jni::AttachCurrentThread();
  BASE_LOG(ERROR) << "JNI_OnLoad";
  return JNI_VERSION_1_4;
}
