#include <jni.h>
#include <string>
#include <openssl/crypto.h>
#include "json/JSON.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_mgg_base_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}