#include <jni.h>

#include <iostream>
#include <string>

#include "version.h"
#include "so_file_version.h"

template <typename T>
int getValueType(T v) {
  if (std::is_same_v<T, int>) {
    return 1;
  }

  if (std::is_same_v<T, double>) {
    return 2;
  }
  if (std::is_same_v<T, char*>) {
    return 3;
  }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_mgg_test_MainActivity_stringFromJNI(JNIEnv* env,
                                                      jobject /* this */) {
  std::string hello = std::string(QCI_BUILD_ID) + std::string(".") +
                      std::string(QCI_JOB_ID) + std::string(".") +
                      std::string(QCI_BUILD_NUMBER) + std::string(GIT_COMMIT_SHA1);
  return env->NewStringUTF(hello.c_str());
}