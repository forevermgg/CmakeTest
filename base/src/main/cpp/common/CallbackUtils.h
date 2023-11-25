#pragma once

#include <jni.h>

#include "CallbackHandler.h"
#include "NioUtils.h"

struct CallbackJni {
#ifdef __ANDROID__
  jclass handlerClass = nullptr;
  jmethodID post = nullptr;
#endif
  jclass executorClass = nullptr;
  jmethodID execute = nullptr;
};

void acquireCallbackJni(JNIEnv* env, CallbackJni& callbackUtils);
void releaseCallbackJni(JNIEnv* env, CallbackJni callbackUtils, jobject handler,
                        jobject callback);

struct JniCallback : private CallbackHandler {
  JniCallback(JniCallback const&) = delete;
  JniCallback(JniCallback&&) = delete;
  JniCallback& operator=(JniCallback const&) = delete;
  JniCallback& operator=(JniCallback&&) = delete;

  // create a JniCallback
  static JniCallback* make(JNIEnv* env, jobject handler, jobject runnable);

  // execute the callback on the java thread and destroy ourselves
  static void postToJavaAndDestroy(JniCallback* callback);

  // CallbackHandler interface.
  void post(void* user, Callback callback) override;

  // Get the CallbackHandler interface
  CallbackHandler* getHandler() noexcept { return this; }

  jobject getCallbackObject() { return mCallback; }

 protected:
  JniCallback(JNIEnv* env, jobject handler, jobject runnable);
  explicit JniCallback() = default;  // this version does nothing
  virtual ~JniCallback();
  jobject mHandler{};
  jobject mCallback{};
  CallbackJni mCallbackUtils{};
};

struct JniBufferCallback : public JniCallback {
  // create a JniBufferCallback
  static JniBufferCallback* make(JNIEnv* env, jobject handler, jobject callback,
                                 AutoBuffer&& buffer);

  // execute the callback on the java thread and destroy ourselves
  static void postToJavaAndDestroy(void*, size_t, void* user);

 private:
  JniBufferCallback(JNIEnv* env, jobject handler, jobject callback,
                    AutoBuffer&& buffer);
  virtual ~JniBufferCallback();
  AutoBuffer mBuffer;
};

struct JniImageCallback : public JniCallback {
  // create a JniImageCallback
  static JniImageCallback* make(JNIEnv* env, jobject handler, jobject runnable,
                                long image);

  // execute the callback on the java thread and destroy ourselves
  static void postToJavaAndDestroy(void*, void* user);

 private:
  JniImageCallback(JNIEnv* env, jobject handler, jobject runnable, long image);
  virtual ~JniImageCallback();
  long mImage;
};
