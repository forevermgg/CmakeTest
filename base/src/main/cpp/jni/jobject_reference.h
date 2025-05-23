#ifndef FOREVER_JOBJECT_REFERENCE_H_
#define FOREVER_JOBJECT_REFERENCE_H_

#include <jni.h>

namespace FOREVER {
namespace INTERNAL {

// Creates an alias of internal::JObjectReference named classname.
// This is useful when defining the implementation of a forward declared class
// using JObjectReference.
#define JOBJECT_REFERENCE(classname)                                        \
  class classname : public firebase::internal::JObjectReference {           \
   public:                                                                  \
    explicit classname(JNIEnv* env)                                         \
        : firebase::internal::JObjectReference(env) {}                      \
    explicit classname(const firebase::internal::JObjectReference& obj)     \
        : firebase::internal::JObjectReference(obj) {}                      \
    explicit classname(firebase::internal::JObjectReference&& obj)          \
        : firebase::internal::JObjectReference(obj) {}                      \
    classname(JNIEnv* env, jobject obj)                                     \
        : firebase::internal::JObjectReference(env, obj) {}                 \
    classname& operator=(const firebase::internal::JObjectReference& rhs) { \
      firebase::internal::JObjectReference::operator=(rhs);                 \
      return *this;                                                         \
    }                                                                       \
    classname& operator=(firebase::internal::JObjectReference&& rhs) {      \
      firebase::internal::JObjectReference::operator=(rhs);                 \
      return *this;                                                         \
    }                                                                       \
  }

// Creates and holds a global reference to a Java object.
class JObjectReference {
 public:
  JObjectReference();
  explicit JObjectReference(JNIEnv* env);
  // Create a reference to a java object.
  JObjectReference(JNIEnv* env, jobject object);
  // Copy
  JObjectReference(const JObjectReference& reference);
  // Move
  JObjectReference(JObjectReference&& reference) noexcept;
  // Delete the reference to the java object.
  ~JObjectReference();
  // Copy this reference.
  JObjectReference& operator=(const JObjectReference& reference);
  // Move this reference.
  JObjectReference& operator=(JObjectReference&& reference) noexcept;

  // Add a global reference to the specified object, removing the reference
  // to the object currently referenced by this class.  If jobject_reference
  // is null, the existing reference is removed.
  void Set(jobject jobject_reference);

  void Set(JNIEnv* env, jobject jobject_reference);

  JNIEnv* GetThreadsafeJNIEnv(JavaVM* java_vm) const;
  // Get a JNIEnv from the JavaVM associated with this class.
  JNIEnv* GetJNIEnv() const;

  // Get the JavaVM associated with this class.
  JavaVM* java_vm() const { return java_vm_; }

  // Get the global reference to the Java object without incrementing the
  // reference count.
  jobject object() const { return object_; }

  // Get a local reference to the object. The returned reference must be
  // deleted after use with DeleteLocalRef().
  jobject GetLocalRef() const;

  // Same as object()
  jobject operator*() const { return object(); }

  // Convert a local reference to a JObjectReference, deleting the local
  // reference.
  static JObjectReference FromLocalReference(JNIEnv* env,
                                             jobject local_reference);

 private:
  // Initialize this instance by adding a reference to the specified Java
  // object.
  void Initialize(JavaVM* jvm, JNIEnv* env, jobject jobject_reference);
  // Get JavaVM from a JNIEnv.
  static JavaVM* GetJavaVM(JNIEnv* env);

 private:
  JavaVM* java_vm_;
  jobject object_;
};

}  // namespace INTERNAL
}  // namespace FOREVER

#endif  // FOREVER_JOBJECT_REFERENCE_H_
