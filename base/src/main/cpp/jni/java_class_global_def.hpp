#ifndef FOREVER_JNI_IMPL_CLASS_GLOBAL_DEF_HPP
#define FOREVER_JNI_IMPL_CLASS_GLOBAL_DEF_HPP

#include <memory>

#include "java_class.hpp"
#include "java_method.hpp"

namespace FOREVER {

namespace _impl {
class JavaClassGlobalDef {
 private:
  JavaClassGlobalDef(JNIEnv* env)
      : m_java_lang_long(env, "java/lang/Long", false),
        m_java_lang_float(env, "java/lang/Float", false),
        m_java_lang_double(env, "java/lang/Double", false),
        m_java_util_date(env, "java/util/Date", false),
        m_java_lang_string(env, "java/lang/String", false),
        m_java_lang_boolean(env, "java/lang/Boolean", false),
        m_java_lang_object(env, "java/lang/Object", false),
        m_forever_notifier(env, "com/test/cmaketest/ForeverNotifier", false),
        m_bson_decimal128(env, "org/bson/types/Decimal128", false),
        m_bson_object_id(env, "org/bson/types/ObjectId", false),
        m_java_util_uuid(env, "java/util/UUID", false) {}

  JNI_UTIL::JavaClass m_java_lang_long;
  JNI_UTIL::JavaClass m_java_lang_float;
  JNI_UTIL::JavaClass m_java_lang_double;
  JNI_UTIL::JavaClass m_java_util_date;
  JNI_UTIL::JavaClass m_java_lang_string;
  JNI_UTIL::JavaClass m_java_lang_boolean;
  JNI_UTIL::JavaClass m_java_lang_object;

  JNI_UTIL::JavaClass m_forever_notifier;
  JNI_UTIL::JavaClass m_bson_decimal128;
  JNI_UTIL::JavaClass m_bson_object_id;
  JNI_UTIL::JavaClass m_java_util_uuid;

  inline static std::unique_ptr<JavaClassGlobalDef>& instance() {
    static std::unique_ptr<JavaClassGlobalDef> instance;
    return instance;
  };

 public:
  // Called in JNI_OnLoad
  static void initialize(JNIEnv* env) {
    // REALM_ASSERT(!instance());
    instance().reset(new JavaClassGlobalDef(env));
  }
  // Called in JNI_OnUnload
  static void release() {
    // REALM_ASSERT(instance());
    instance().release();
  }

  // java.lang.Long
  inline static jobject new_long(JNIEnv* env, int64_t value) {
    static JNI_UTIL::JavaMethod init(env, instance()->m_java_lang_long,
                                     "<init>", "(J)V");
    return env->NewObject(instance()->m_java_lang_long, init, value);
  }
  inline static const JNI_UTIL::JavaClass& java_lang_long() {
    return instance()->m_java_lang_long;
  }

  // java.lang.Float
  inline static jobject new_float(JNIEnv* env, float value) {
    static JNI_UTIL::JavaMethod init(env, instance()->m_java_lang_float,
                                     "<init>", "(F)V");
    return env->NewObject(instance()->m_java_lang_float, init, value);
  }
  inline static const JNI_UTIL::JavaClass& java_lang_float() {
    return instance()->m_java_lang_float;
  }

  // java.lang.Double
  inline static jobject new_double(JNIEnv* env, double value) {
    static JNI_UTIL::JavaMethod init(env, instance()->m_java_lang_double,
                                     "<init>", "(D)V");
    return env->NewObject(instance()->m_java_lang_double, init, value);
  }
  inline static const JNI_UTIL::JavaClass& java_lang_double() {
    return instance()->m_java_lang_double;
  }

  // java.lang.Boolean
  inline static jobject new_boolean(JNIEnv* env, bool value) {
    static JNI_UTIL::JavaMethod init(env, instance()->m_java_lang_boolean,
                                     "<init>", "(Z)V");
    return env->NewObject(instance()->m_java_lang_boolean, init,
                          value ? JNI_TRUE : JNI_FALSE);
  }
  inline static const JNI_UTIL::JavaClass& java_lang_boolean() {
    return instance()->m_java_lang_boolean;
  }

  inline static const JNI_UTIL::JavaClass& java_util_date() {
    return instance()->m_java_util_date;
  }

  // java.util.String
  inline static const JNI_UTIL::JavaClass& java_lang_string() {
    return instance()->m_java_lang_string;
  }

  // java.lang.Object
  inline static const JNI_UTIL::JavaClass& java_lang_object() {
    return instance()->m_java_lang_object;
  }

  // io.realm.internal.RealmNotifier
  inline static const JNI_UTIL::JavaClass& forever_notifier() {
    return instance()->m_forever_notifier;
  }
};

}  // namespace _impl
}  // namespace FOREVER

#endif  // FOREVER_JNI_IMPL_CLASS_GLOBAL_DEF_HPP
