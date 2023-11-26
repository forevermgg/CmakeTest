#ifndef FOREVER_UTIL_ANDROID_H_
#define FOREVER_UTIL_ANDROID_H_

#include <jni.h>
#include <stddef.h>

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "../log_level.h"
#include "../log_settings.h"
#include "../logging.h"
#include "../variant/variant.h"
#include "embedded_file.h"
#include "jobject_reference.h"

// To ensure that Proguard doesn't strip the classes you're using, place this
// string directly before the JNI class string in your METHOD_LOOKUP_DEFINITION
// macro invocation.
#define PROGUARD_KEEP_CLASS "%PG%"

namespace FOREVER {
typedef struct {
  const char* name;
  const char* signature;
  void* fnPtr;  // NOLINT
} JNINativeMethod;
// Converts FOREVER::JNINativeMethod array to ::JNINativeMethod
// array.
// Caller ownes the result and must call CleanUpConvertedJNINativeMethod() to
// release the allocation.
inline const ::JNINativeMethod* ConvertJNINativeMethod(
    const JNINativeMethod* source, size_t number_of_native_methods) {
  ::JNINativeMethod* target = new ::JNINativeMethod[number_of_native_methods];
  ::JNINativeMethod* result = target;
  for (int i = 0; i < number_of_native_methods; ++i) {
    target->name = const_cast<char*>(source->name);
    target->signature = const_cast<char*>(source->signature);
    target->fnPtr = source->fnPtr;
    ++source;
    ++target;
  }
  return result;
}
inline void CleanUpConvertedJNINativeMethod(const ::JNINativeMethod* target) {
  // It is not const; we created it in ConvertJNINativeMethod().
  delete[] const_cast<::JNINativeMethod*>(target);
}

namespace UTIL {

// Type of a Java method.
enum MethodType {
  kMethodTypeInstance,  // Called on an instance of a class.
  kMethodTypeStatic,    // Static method of a class.
};

// Whether a Java method (or field) is required or optional.
enum MethodRequirement {
  kMethodRequired,  // The method is required, an assert will occur if you try
                    // to look it up and it doesn't exist.
  kMethodOptional,  // The method is optional, no error will occur if you try
                    // to look it up and it doesn't exist.
};

// Name and signature of a class method.
struct MethodNameSignature {
  const char* name;       // Name of the method e.g java/lang/Object
  const char* signature;  // JNI signature of the method e.g "()I"
  const MethodType type;
  const MethodRequirement optional;  // Whether lookup of this method should
                                     // fail silently.
};

// Whether this is instance or static field in a class.
enum FieldType {
  kFieldTypeInstance,  // Field on an instance of a class.
  kFieldTypeStatic,    // Static field of a class.
};

// Name and type of a class field.
struct FieldDescriptor {
  const char* name;       // Name of the field.
  const char* signature;  // JNI string that describes the type.
  const FieldType type;
  const MethodRequirement optional;  // Whether lookup of this field should fail
                                     // silently.
};

// Whether a Java class is required or optional.
enum ClassRequirement {
  kClassRequired,  // The class is required, an error will be logged if you
                   // try to look it up and it doesn't exist.
  kClassOptional,  // The class is optional, no error be logged if you try
                   // to look it up and it doesn't exist.
};

// Maps ResourceType enumeration values to strings.
// clang-format off
#define RESOURCE_TYPES(X) \
  X(String, "string"), \
  X(Id, "id")
// clang-format on
#define RESOURCE_TYPE_ENUM(type_identifier, type_name) \
  kResourceType##type_identifier
#define RESOURCE_TYPE_STRING(type_identifier, type_name) type_name

// Android Resource types.
// See http://developer.android.com/guide/topics/resources/
// NOTE: This is not an exhaustive set of all resource types.
enum ResourceType { RESOURCE_TYPES(RESOURCE_TYPE_ENUM) };

// Initialize the utilities library. It is safe to call this multiple times
// though each call should be paired with a Terminate() as the initialization
// is reference counted.
bool Initialize(JNIEnv* env, jobject activity_object);

// This causes only a partial initialization of this module. It can be used
// instead of "Initialize" and will support only very basic use of "FindClass",
// and none of the other API methods in this module. This is highly specialized
// to provide only the Activity Class Loader. It's intended to be used when
// only the Activity Class Loader is needed, and in that case can initialize
// much more quickly as the full initialization requires reading other classes
// from the disk and compiling them.
// Calls to InitializeActivityClasses should be paired with a corresponding call
// to TerminateActivityClasses.
bool InitializeActivityClasses(JNIEnv* env, jobject activity_object);

// Terminate the utilities library.  Releases all global references to
// classes.
void Terminate(JNIEnv* env);

// Terminate the activity class loader that was setup with
// InitializeActivityClasses.
void TerminateActivityClasses(JNIEnv* env);

// Lookup method IDs specified by the method_name_signatures array and store
// in method_ids.  Used by METHOD_LOOKUP_DECLARATION.
bool LookupMethodIds(JNIEnv* env, jclass clazz,
                     const MethodNameSignature* method_name_signatures,
                     size_t number_of_method_name_signatures,
                     jmethodID* method_ids, const char* class_name);

// Lookup field IDs specified by the field_descriptors array and store
// in field_ids.  Used by FIELD_LOOKUP_DECLARATION.
bool LookupFieldIds(JNIEnv* env, jclass clazz,
                    const FieldDescriptor* field_descriptors,
                    size_t number_of_field_descriptors, jfieldID* field_ids,
                    const char* class_name);

// Used to make METHOD_ID and METHOD_NAME_SIGNATURE macros variadic.
#define METHOD_NAME_INFO_EXPANDER(arg0, arg1, arg2, arg3, arg4, arg5, \
                                  function, ...)                      \
  function

// Used to populate an array of MethodNameSignature.
#define METHOD_NAME_SIGNATURE_5(id, name, signature, method_type, optional) \
  { name, signature, method_type, optional }
#define METHOD_NAME_SIGNATURE_4(id, name, signature, method_type) \
  METHOD_NAME_SIGNATURE_5(id, name, signature, method_type,       \
                          ::FOREVER::UTIL::MethodRequirement::kMethodRequired)
#define METHOD_NAME_SIGNATURE_3(id, name, signature) \
  METHOD_NAME_SIGNATURE_4(id, name, signature,       \
                          ::FOREVER::UTIL::kMethodTypeInstance)
#define METHOD_NAME_SIGNATURE(...)                                        \
  METHOD_NAME_INFO_EXPANDER(, ##__VA_ARGS__,                              \
                            METHOD_NAME_SIGNATURE_5(__VA_ARGS__),         \
                            METHOD_NAME_SIGNATURE_4(__VA_ARGS__),         \
                            METHOD_NAME_SIGNATURE_3(__VA_ARGS__),         \
                            numargs2(__VA_ARGS__), numargs1(__VA_ARGS__), \
                            /* Generate nothing with no args. */)

// Used to populate an enum of method identifiers.
#define METHOD_ID_3(id, name, signature) k##id
#define METHOD_ID_4(id, name, signature, method_type) k##id
#define METHOD_ID_5(id, name, signature, method_type, optional) k##id
#define METHOD_ID(...)                                                        \
  METHOD_NAME_INFO_EXPANDER(                                                  \
      , ##__VA_ARGS__, METHOD_ID_5(__VA_ARGS__), METHOD_ID_4(__VA_ARGS__),    \
      METHOD_ID_3(__VA_ARGS__), numargs2(__VA_ARGS__), numargs1(__VA_ARGS__), \
      /* Generate nothing with no args. */)

// Used to populate FieldDescriptor
#define FIELD_DESCRIPTOR(...) METHOD_NAME_SIGNATURE(__VA_ARGS__)
// Used to populate an enum of field identifiers.
#define FIELD_ID(...) METHOD_ID(__VA_ARGS__)

// Used with METHOD_LOOKUP_DECLARATION to generate no method lookups.
#define METHOD_LOOKUP_NONE(X)                       \
  X(InvalidMethod, nullptr, nullptr,                \
    ::FOREVER::UTIL::MethodType::kMethodTypeStatic, \
    ::FOREVER::UTIL::MethodRequirement::kMethodOptional)

// Used with METHOD_LOOKUP_DECLARATION to generate no field lookups.
#define FIELD_LOOKUP_NONE(X)                      \
  X(InvalidField, nullptr, nullptr,               \
    ::FOREVER::UTIL::FieldType::kFieldTypeStatic, \
    ::FOREVER::UTIL::MethodRequirement::kMethodOptional)

// Declares a namespace which caches class method IDs.
// To make cached method IDs available in to other files or projects, this macro
// must be defined in a header file. If the method IDs are local to a specific
// file however, this can be placed in the source file. Regardless of whether
// this is placed in the header or not, you must also add
// METHOD_LOOKUP_DEFINITION to the source file.
// clang-format off
#define METHOD_LOOKUP_DECLARATION_3(namespace_identifier,                     \
                                    method_descriptor_macro,                  \
                                    field_descriptor_macro)                   \
  namespace namespace_identifier {                                            \
                                                                              \
  enum Method {                                                               \
    method_descriptor_macro(METHOD_ID),                                       \
    kMethodCount                                                              \
  };                                                                          \
                                                                              \
  enum Field {                                                                \
    field_descriptor_macro(FIELD_ID),                                         \
    kFieldCount                                                               \
  };                                                                          \
                                                                              \
  /* Find and hold a reference to this namespace's class, optionally */       \
  /* searching a set of files for the class. */                               \
  /* This specified file list must have previously been cached using */       \
  /* CacheEmbeddedFiles(). */                                                 \
  jclass CacheClassFromFiles(                                                 \
      JNIEnv *env, jobject activity_object,                                   \
      const std::vector<::FOREVER::INTERNAL::EmbeddedFile>*        \
          embedded_files);                                                    \
                                                                              \
  /* Find and hold a reference to this namespace's class. */                  \
  jclass CacheClass(JNIEnv *env);                                             \
                                                                              \
  /* Get the cached class associated with this namespace. */                  \
  jclass GetClass();                                                          \
                                                                              \
  /* Register native methods on the class associated with this namespace. */  \
  bool RegisterNatives(JNIEnv* env, const JNINativeMethod* native_methods,    \
                       size_t number_of_native_methods);                      \
                                                                              \
  /* Release the cached class reference. */                                   \
  void ReleaseClass(JNIEnv *env);                                             \
                                                                              \
  /* See LookupMethodIds() */                                                 \
  bool CacheMethodIds(JNIEnv *env, jobject activity_object);                  \
                                                                              \
  /* Lookup a method ID using a Method enum value. */                         \
  jmethodID GetMethodId(Method method);                                       \
                                                                              \
  /* See LookupFieldIds() */                                                  \
  bool CacheFieldIds(JNIEnv *env, jobject activity_object);                   \
                                                                              \
  /* Lookup a field ID using a Field enum value. */                           \
  jfieldID GetFieldId(Field field);                                           \
                                                                              \
  }  // namespace namespace_identifier

#define METHOD_LOOKUP_DECLARATION_2(namespace_identifier,                     \
                                    method_descriptor_macro)                  \
  METHOD_LOOKUP_DECLARATION_3(namespace_identifier, method_descriptor_macro,  \
                              FIELD_LOOKUP_NONE)

// Used to make METHOD_LOOKUP_DECLARATION variadic.
#define METHOD_LOOKUP_DECLARATION_EXPANDER(arg0, arg1, arg2, arg3,            \
                                           function, ...)                     \
  function

#define METHOD_LOOKUP_DECLARATION(...)                                        \
  METHOD_LOOKUP_DECLARATION_EXPANDER(                                         \
      , ##__VA_ARGS__, METHOD_LOOKUP_DECLARATION_3(__VA_ARGS__),              \
      METHOD_LOOKUP_DECLARATION_2(__VA_ARGS__), numargs1(__VA_ARGS__),        \
      numargs0(__VA_ARGS__))

// Defines a namespace which caches class method IDs.
// To cache class method IDs, you must first declare them in either the header
// or source file with METHOD_LOOKUP_DECLARATION. Regardless of whether the
// declaration is in a header or source file, this macro must be called from a
// source file.
// clang-format off
#define METHOD_LOOKUP_DEFINITION_4(namespace_identifier,                      \
                                   class_name, method_descriptor_macro,       \
                                   field_descriptor_macro)                    \
  namespace namespace_identifier {                                            \
                                                                              \
  /* Skip optional "%PG%" at the beginning of the string, if present. */      \
  static const char* kClassName =                                             \
      class_name[0] == '%' ? &class_name[4] : class_name;                     \
                                                                              \
  static const ::FOREVER::UTIL::MethodNameSignature                \
      kMethodSignatures[] = {                                                 \
    method_descriptor_macro(METHOD_NAME_SIGNATURE),                           \
  };                                                                          \
                                                                              \
  static const ::FOREVER::UTIL::FieldDescriptor                    \
      kFieldDescriptors[] = {                                                 \
    field_descriptor_macro(FIELD_DESCRIPTOR),                                 \
  };                                                                          \
                                                                              \
  static jmethodID g_method_ids[kMethodCount];                                \
  static jfieldID g_field_ids[kFieldCount];                                   \
                                                                              \
  static jclass g_class = nullptr;                                            \
  static bool g_registered_natives = false;                                   \
                                                                              \
  /* Find and hold a reference to this namespace's class, optionally */       \
  /* searching a set of files for the class. */                               \
  /* This specified file list must have previously been cached using */       \
  /* CacheEmbeddedFiles(). */                                                 \
  /* If optional == kClassOptional, no errors will be emitted if the class */ \
  /* does not exist. */                                                       \
  jclass CacheClassFromFiles(                                                 \
      JNIEnv *env, jobject activity_object,                                   \
      const std::vector<::FOREVER::INTERNAL::EmbeddedFile>*        \
          embedded_files,                                                     \
      ::FOREVER::UTIL::ClassRequirement optional) {                \
    if (!g_class) {                                                           \
      g_class = ::FOREVER::UTIL::FindClassGlobal(                  \
          env, activity_object, embedded_files, kClassName, optional);        \
    }                                                                         \
    return g_class;                                                           \
  }                                                                           \
                                                                              \
  jclass CacheClassFromFiles(                                                 \
      JNIEnv *env, jobject activity_object,                                   \
      const std::vector<::FOREVER::INTERNAL::EmbeddedFile>*        \
          embedded_files) {                                                   \
    return CacheClassFromFiles(                                               \
        env, activity_object, embedded_files,                                 \
        ::FOREVER::UTIL::ClassRequirement::kClassRequired);        \
  }                                                                           \
                                                                              \
  /* Find and hold a reference to this namespace's class. */                  \
  jclass CacheClass(JNIEnv* env, jobject activity_object,                     \
                    ::FOREVER::UTIL::ClassRequirement optional) {  \
    return CacheClassFromFiles(env, activity_object, nullptr, optional);      \
  }                                                                           \
                                                                              \
  /* Find and hold a reference to this namespace's class. */                  \
  jclass CacheClass(JNIEnv* env, jobject activity_object) {                   \
    return CacheClassFromFiles(                                               \
        env, activity_object, nullptr,                                        \
        ::FOREVER::UTIL::ClassRequirement::kClassRequired);        \
  }                                                                           \
                                                                              \
  /* Get the cached class associated with this namespace. */                  \
  jclass GetClass() { return g_class; }                                       \
                                                                              \
  bool RegisterNatives(JNIEnv* env, const JNINativeMethod* native_methods,    \
                       size_t number_of_native_methods) {                     \
    if (g_registered_natives) return false;                                   \
    const ::JNINativeMethod* true_native_methods =                            \
        FOREVER::ConvertJNINativeMethod(native_methods,            \
                                         number_of_native_methods);           \
    const jint register_status = env->RegisterNatives(                        \
      GetClass(), true_native_methods, number_of_native_methods);             \
    FOREVER::CleanUpConvertedJNINativeMethod(true_native_methods); \
    FOREVER::util::CheckAndClearJniExceptions(env);                \
    g_registered_natives = register_status == JNI_OK;                         \
    return g_registered_natives;                                              \
  }                                                                           \
                                                                              \
  /* Release the cached class reference. */                                   \
  void ReleaseClass(JNIEnv* env) {                                            \
    if (g_class) {                                                            \
      if (g_registered_natives) {                                             \
        env->UnregisterNatives(g_class);                                      \
        g_registered_natives = false;                                         \
      }                                                                       \
      FOREVER::util::CheckAndClearJniExceptions(env);              \
      env->DeleteGlobalRef(g_class);                                          \
      g_class = nullptr;                                                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  /* See LookupMethodIds() */                                                 \
  /* If the class is being loaded from an embedded file use */                \
  /* CacheClassFromFiles() before calling this function to cache method */    \
  /* IDs. */                                                                  \
  bool CacheMethodIds(JNIEnv* env, jobject activity_object) {                 \
    return ::FOREVER::UTIL::LookupMethodIds(                       \
        env, CacheClass(env, activity_object), kMethodSignatures,             \
        kMethodCount, g_method_ids, kClassName);                              \
  }                                                                           \
                                                                              \
  /* Lookup a method ID using a Method enum value. */                         \
  jmethodID GetMethodId(Method method) {                                      \
    // FIREBASE_ASSERT(method < kMethodCount);                                \
    jmethodID method_id = g_method_ids[method];                               \
    return method_id;                                                         \
  }                                                                           \
                                                                              \
  /* See LookupFieldIds() */                                                  \
  /* If the class is being loaded from an embedded file use */                \
  /* CacheClassFromFiles() before calling this function to cache field */     \
  /* IDs. */                                                                  \
  bool CacheFieldIds(JNIEnv* env, jobject activity_object) {                  \
    return ::FOREVER::UTIL::LookupFieldIds(                        \
        env, CacheClass(env, activity_object),                                \
        kFieldDescriptors, kFieldCount,                                       \
        g_field_ids, kClassName);                                             \
  }                                                                           \
                                                                              \
  /* Lookup a field ID using a Field enum value. */                           \
  jfieldID GetFieldId(Field field) {                                          \
    // FIREBASE_ASSERT(field < kFieldCount);                                  \
    jfieldID field_id = g_field_ids[field];                                   \
    return field_id;                                                          \
  }                                                                           \
                                                                              \
  }  // namespace namespace_identifier
// clang-format on

#define METHOD_LOOKUP_DEFINITION_3(namespace_identifier, class_name, \
                                   method_descriptor_macro)          \
  METHOD_LOOKUP_DEFINITION_4(namespace_identifier, class_name,       \
                             method_descriptor_macro, FIELD_LOOKUP_NONE)

// Used to make METHOD_LOOKUP_DEFINITION variadic.
#define METHOD_LOOKUP_DEFINITION_EXPANDER(arg0, arg1, arg2, arg3, arg4, \
                                          function, ...)                \
  function

#define METHOD_LOOKUP_DEFINITION(...)                                 \
  METHOD_LOOKUP_DEFINITION_EXPANDER(                                  \
      , ##__VA_ARGS__, METHOD_LOOKUP_DEFINITION_4(__VA_ARGS__),       \
      METHOD_LOOKUP_DEFINITION_3(__VA_ARGS__), numargs2(__VA_ARGS__), \
      numargs1(__VA_ARGS__), numargs0(__VA_ARGS))

}  // namespace UTIL
// NOLINTNEXTLINE - allow namespace overridden
}  // namespace FOREVER

#endif  // FOREVER_UTIL_ANDROID_H_
