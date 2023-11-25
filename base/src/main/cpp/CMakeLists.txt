cmake_minimum_required(VERSION 3.22.1)

project("base")

set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/prefix/openssl)
message("OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")
include_directories(${OPENSSL_ROOT_DIR}/${ANDROID_ABI}/include)
message("OPENSSL_INCLUDE_DIR: ${OPENSSL_ROOT_DIR}/${ANDROID_ABI}/include")
# 添加两个预编译库
add_library(openssl-crypto
        STATIC
        IMPORTED)

set_target_properties(openssl-crypto
        PROPERTIES IMPORTED_LOCATION
        ${OPENSSL_ROOT_DIR}/${ANDROID_ABI}/lib/libcrypto.a)

add_library(openssl-ssl
        STATIC
        IMPORTED)

set_target_properties(openssl-ssl
        PROPERTIES IMPORTED_LOCATION
        ${OPENSSL_ROOT_DIR}/${ANDROID_ABI}/lib/libssl.a)

set(LIBUV_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/prefix/libuv)
message("LIBUV_ROOT_DIR: ${LIBUV_ROOT_DIR}")
include_directories(${LIBUV_ROOT_DIR}/${ANDROID_ABI}/include)
message("LIBUV_INCLUDE_DIR: ${LIBUV_ROOT_DIR}/${ANDROID_ABI}/include")

add_library(libuv
        STATIC
        IMPORTED)

set_target_properties(libuv
        PROPERTIES IMPORTED_LOCATION
        ${LIBUV_ROOT_DIR}/${ANDROID_ABI}/lib/libuv.a)


set(JSON_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/json/json)
message("JSON_ROOT_DIR: ${JSON_ROOT_DIR}")
add_subdirectory(${JSON_ROOT_DIR}  ${JSON_ROOT_DIR}/build)
include_directories(${JSON_ROOT_DIR}/json/single_include)

set(ABSEIL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/abseil-cpp)
message("ABSEIL_ROOT_DIR: ${ABSEIL_ROOT_DIR}")
add_subdirectory(${ABSEIL_ROOT_DIR}  ${ABSEIL_ROOT_DIR}/build)


set(LIBUV_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/libuv)
message("LIBUV_ROOT_DIR: ${LIBUV_ROOT_DIR}")
add_subdirectory(${LIBUV_ROOT_DIR}  ${LIBUV_ROOT_DIR}/build)

include_directories(json/*.h)
include_directories(string/*.h)

add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        jni/string/jni_string.h
        jni/string/jni_string.cpp
        jni/jni_util.cc
        jni/jni_util.h
        jni/jni_weak_ref.cc
        jni/jni_weak_ref.h
        jni/scoped_java_ref.cc
        jni/scoped_java_ref.h

        json/JSON.h
        json/JSON.cpp
        json/JSONGenerator.h
        json/same.h
        json/same.cpp

        string/ref_counted_string.h
        string/ref_counted_string.cc
        string/StdStringExtractor.h
        string/StdStringExtractor.cpp
        string/string_convert.h
        string/string_convert.cpp

        synchronization/atomic_object.h
        synchronization/count_down_latch.h
        synchronization/count_down_latch.cc
        synchronization/semaphore.h
        synchronization/semaphore.cc
        synchronization/shared_mutex.h
        synchronization/shared_mutex_std.h
        synchronization/shared_mutex_std.cc
        synchronization/sync_switch.h
        synchronization/sync_switch.cc
        synchronization/waitable_event.h
        synchronization/waitable_event.cc

        time/chrono_timestamp_provider.h
        time/chrono_timestamp_provider.cc
        time/time_delta.h
        time/time_point.h
        time/time_point.cc
        time/timestamp_provider.h

        util/android/scheduler.hpp
        util/generic/scheduler.hpp
        util/uv/scheduler.hpp
        util/event_loop.hpp
        util/event_loop.cpp
        util/event_loop_dispatcher.hpp
        util/optional.hpp
        util/scheduler.cpp
        util/scheduler.hpp
        util/scope_exit.hpp
        util/functional.hpp
        util/function_ref.hpp
        util/type_traits.hpp
        util/version_id.hpp

        base.cpp
        base32.cc
        base32.h
        base64.cc
        base64.h
        eintr_wrapper.h

        library_loader.cc

        log_level.h
        log_settings.cc
        log_settings.h
        log_settings_state.cc
        logging.cc
        logging.h
        macros.h

        string_conversion.cc
        string_conversion.h
        thread_local.cc
        thread_local.h
        )

target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        openssl-ssl openssl-crypto
        nlohmann_json::nlohmann_json
        absl::base
        absl::algorithm
        absl::cleanup
        # absl::container
        # absl::crc
        absl::debugging
        absl::flags
        absl::hash
        absl::log
        absl::memory
        absl::numeric
        # absl::profiling
        # absl::random
        absl::status
        absl::strings
        absl::synchronization
        absl::time
        # absl::types
        absl::utility
        libuv
        )