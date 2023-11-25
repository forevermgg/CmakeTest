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
    FOREVER::_impl::JavaClassGlobalDef::initialize(env);
  }
  VirtualMachineEnv::JNI_OnLoad(vm);
  return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNI_OnUnload(JavaVM* vm, void*) {
  JNIEnv* env;
  if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
    return;
  } else {
    FOREVER::_impl::JavaClassGlobalDef::release();
    FOREVER::JNI_UTIL::JniUtils::release();
  }
}
