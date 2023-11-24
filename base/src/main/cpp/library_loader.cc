
// This is called by the VM when the shared library is first loaded.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  // Initialize the Java VM.
  BASE::jni::InitJavaVM(vm);

  JNIEnv* env = BASE::jni::AttachCurrentThread();
  bool result = false;

  return JNI_VERSION_1_4;
}
