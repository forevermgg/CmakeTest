//
// Created by centforever on 2023/11/25.
//
#ifndef BASE_JNI_STRING_H
#define BASE_JNI_STRING_H

#include <jni.h>

#include <string>

namespace base {
namespace jni {
namespace string {

// Converts a jstring to a std::string in UTF8 and releases the memory of the
// jstring.
std::string ConvertJavaStringToUTF8(JNIEnv *env, jstring java_string);

// Converts a jbyteArray to a std::string in UTF8 and releases the memory of the
// jbyteArray.
std::string ConvertJavaByteArrayToString(JNIEnv *env, jbyteArray byte_array);

}  // namespace string
}  // namespace jni
}  // namespace base

#endif  // BASE_JNI_STRING_H
