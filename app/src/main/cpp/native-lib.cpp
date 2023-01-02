#include <jni.h>

#include <iostream>
#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

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
Java_com_test_buglyversion_MainActivity_stringFromJNI(JNIEnv* env,
                                                      jobject /* this */) {
  spdlog::set_level(spdlog::level::debug); // Set global log level to debug
  std::string hello = std::string(QCI_BUILD_ID) + std::string(".") +
                      std::string(QCI_JOB_ID) + std::string(".") +
                      std::string(QCI_BUILD_NUMBER) + std::string(GIT_COMMIT_SHA1);
  std::string msg;
  //EXPECT_EQ(msg, nullptr);
  //EXPECT_EQ(msg, "");
  nlohmann::json object = {{"one", 1}, {"two", 2}};
  getValueType(1);
  spdlog::error("Some error message with arg: {}", 1);
  // change log pattern
  spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
  return env->NewStringUTF(hello.c_str());
}