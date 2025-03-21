//
// Created by centforever on 2023/11/25.
//

#include "jni_string.h"

#include <jni.h>

#include <string>

namespace base {
namespace jni {
namespace string {

std::string ConvertJavaStringToUTF8(JNIEnv* env, jstring java_string) {
  // Convert Java string to std::string
  const jsize strLen = env->GetStringUTFLength(java_string);
  const char* char_buffer = env->GetStringUTFChars(java_string, nullptr);
  std::string str(char_buffer, strLen);

  // Release memory
  env->ReleaseStringUTFChars(java_string, char_buffer);
  env->DeleteLocalRef(java_string);
  return str;
}

std::string ConvertJavaByteArrayToString(JNIEnv* env, jbyteArray byte_array) {
  // Convert Java byte[] to std::string
  jbyte* bytes = env->GetByteArrayElements(byte_array, nullptr);
  jsize bytes_size = env->GetArrayLength(byte_array);
  std::string byte_string(reinterpret_cast<const char*>(bytes), bytes_size);

  // Release memory
  env->ReleaseByteArrayElements(byte_array, bytes, 0);
  env->DeleteLocalRef(byte_array);
  return byte_string;
}

}  // namespace string
}  // namespace jni
}  // namespace base