#include <jni.h>
#include "jni/jni_util.h"
#include "log_settings.h"
#include "logging.h"
#include "util/event_loop.hpp"
#include "util/scope_exit.hpp"
#include "util/uv/scheduler.hpp"
#include "jni/java_class_global_def.hpp"
#include "jni/jni_utils.hpp"
#include "common/VirtualMachineEnv.h"
#include "absl/log/log_entry.h"
#include <android/log.h>
#include <absl/base/log_severity.h>


void AndroidLogSink(void*, const absl::LogEntry& entry) {
    int androidLogPriority = ANDROID_LOG_UNKNOWN;

    switch (entry.log_severity()) {
        case absl::LogSeverity::kFatal:
            androidLogPriority = ANDROID_LOG_FATAL;
            break;
        case absl::LogSeverity::kError:
            androidLogPriority = ANDROID_LOG_ERROR;
            break;
        case absl::LogSeverity::kWarning:
            androidLogPriority = ANDROID_LOG_WARN;
            break;
        case absl::LogSeverity::kInfo:
            androidLogPriority = ANDROID_LOG_INFO;
            break;
        /*case absl::LogSeverity::kDebug:
            androidLogPriority = ANDROID_LOG_DEBUG;
            break;*/
        default:
            androidLogPriority = ANDROID_LOG_VERBOSE;
    }

    __android_log_print(androidLogPriority, "YOUR_TAG", "%s", entry.text_message_with_prefix_and_newline_c_str());
}

// This is called by the VM when the shared library is first loaded.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::LogSettings log_settings;
  log_settings.min_log_level = base::kLogInfo;
  base::SetLogSettings(log_settings);
  // Initialize the Java VM.
  base::jni::InitJavaVM(vm);
  base::util::Scheduler::set_default_factory([]() {
    return std::make_shared<base::util::UvMainLoopScheduler>();
  });
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
  bool called = false;
  auto handler = [&]() noexcept
  {
      called = true;
  };
  {
    auto seg = base::util::make_scope_exit(handler);
    BASE_LOG(ERROR) << "called = " << called;
  }
  BASE_LOG(ERROR) << "called = " << called;

  if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  } else {
    FOREVER::JNI_UTIL::JniUtils::initialize(vm, JNI_VERSION_1_6);
    // FOREVER::_impl::JavaClassGlobalDef::initialize(env);
  }
  VirtualMachineEnv::JNI_OnLoad(vm);
  return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNI_OnUnload(JavaVM* vm, void*) {
  JNIEnv* env;
  if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
    return;
  } else {
    // FOREVER::_impl::JavaClassGlobalDef::release();
    FOREVER::JNI_UTIL::JniUtils::release();
  }
}
