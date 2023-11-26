#include "jobject_reference.h"

#include <jni.h>

#include "../jni/jni_util.h"

namespace FOREVER {
namespace INTERNAL {

// Static variables used in tracking thread initialization and cleanup.
pthread_key_t jni_env_key;
pthread_once_t pthread_key_initialized = PTHREAD_ONCE_INIT;

namespace {

extern "C" void DetachJVMThreads(void* stored_java_vm) {
  JNIEnv* jni_env;
  JavaVM* java_vm = static_cast<JavaVM*>(stored_java_vm);
  // AttachCurrentThread does nothing if we're already attached, but
  // calling it ensures that the DetachCurrentThread doesn't fail.
#if defined(__ANDROID__)
  java_vm->AttachCurrentThread(&jni_env, nullptr);
#else
  java_vm->AttachCurrentThread(reinterpret_cast<void**>(&jni_env), nullptr);
#endif
  java_vm->DetachCurrentThread();
}

// Called the first time GetJNIEnv is invoked.
// Ensures that jni_env_key is created and that the destructor is in place.
extern "C" void SetupJvmDetachOnThreadDestruction() {
  pthread_key_create(&jni_env_key, DetachJVMThreads);
}

}  // namespace

JObjectReference::JObjectReference() : java_vm_(nullptr), object_(nullptr) {}

JObjectReference::JObjectReference(JNIEnv* env)
    : java_vm_(GetJavaVM(env)), object_(nullptr) {}

JObjectReference::JObjectReference(JNIEnv* env, jobject object) {
  Initialize(GetJavaVM(env), env, object);
}

JObjectReference::JObjectReference(const JObjectReference& reference) {
  Initialize(reference.java_vm_, reference.GetJNIEnv(), reference.object());
}

JObjectReference::JObjectReference(JObjectReference&& reference) noexcept {
  operator=(std::move(reference));
}

JObjectReference::~JObjectReference() { Set(nullptr); }

JObjectReference& JObjectReference::operator=(
    const JObjectReference& reference) {
  Set(reference.GetJNIEnv(), reference.object_);
  return *this;
}

JObjectReference& JObjectReference::operator=(
    JObjectReference&& reference) noexcept {
  java_vm_ = reference.java_vm_;
  object_ = reference.object_;
  reference.java_vm_ = nullptr;
  reference.object_ = nullptr;
  return *this;
}

void JObjectReference::Set(jobject jobject_reference) {
  JNIEnv* env = GetJNIEnv();
  if (env && object_) env->DeleteGlobalRef(object_);
  object_ = nullptr;
  Initialize(java_vm_, env, jobject_reference);
}

void JObjectReference::Set(JNIEnv* env, jobject jobject_reference) {
  if (env && object_) env->DeleteGlobalRef(object_);
  object_ = nullptr;
  Initialize(GetJavaVM(env), env, jobject_reference);
}

JNIEnv* JObjectReference::GetThreadsafeJNIEnv(JavaVM* java_vm) const {
  // Set up the thread key and destructor the first time this is called:
  (void)pthread_once(&pthread_key_initialized,
                     SetupJvmDetachOnThreadDestruction);
  pthread_setspecific(jni_env_key, java_vm);

  JNIEnv* env;
  jint result =
#if defined(__ANDROID__)
      java_vm->AttachCurrentThread(&env, nullptr);
#else
      java_vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
#endif
  return result == JNI_OK ? env : nullptr;
}

JNIEnv* JObjectReference::GetJNIEnv() const {
  return java_vm_ ? GetThreadsafeJNIEnv(java_vm_) : nullptr;
}

jobject JObjectReference::GetLocalRef() const {
  JNIEnv* env = GetJNIEnv();
  return object_ && env ? env->NewLocalRef(object_) : nullptr;
}

JObjectReference JObjectReference::FromLocalReference(JNIEnv* env,
                                                      jobject local_reference) {
  JObjectReference jobject_reference = JObjectReference(env, local_reference);
  if (local_reference) env->DeleteLocalRef(local_reference);
  return jobject_reference;
}

void JObjectReference::Initialize(JavaVM* jvm, JNIEnv* env,
                                  jobject jobject_reference) {
  // FIREBASE_DEV_ASSERT(env || !jobject_reference);
  java_vm_ = jvm;
  object_ = nullptr;
  if (jobject_reference) object_ = env->NewGlobalRef(jobject_reference);
}

JavaVM* JObjectReference::GetJavaVM(JNIEnv* env) {
  // FIREBASE_DEV_ASSERT(env);
  JavaVM* jvm = nullptr;
  jint result = env->GetJavaVM(&jvm);
  // FIREBASE_DEV_ASSERT(result == JNI_OK);
  return jvm;
}

}  // namespace INTERNAL
}  // namespace FOREVER
