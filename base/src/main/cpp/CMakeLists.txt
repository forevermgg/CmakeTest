cmake_minimum_required(VERSION 3.22.1)

project("base")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
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


set(FLATBUFFERS_SRC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/flatbuffers/include)
message("FLATBUFFERS_SRC_INCLUDE ${FLATBUFFERS_SRC_INCLUDE}")
add_subdirectory(flatbuffers)
include_directories(${FLATBUFFERS_SRC_INCLUDE})
FILE(GLOB Generated_SRCS generated/*.h)


find_package(curl REQUIRED CONFIG)
find_package(jsoncpp REQUIRED CONFIG)

add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        common/CallbackHandler.h
        common/CallbackHandler.cpp
        common/CallbackUtils.h
        common/CallbackUtils.cpp
        common/compiler.h
        common/debug.cpp
        common/debug.h
        common/macros.h
        common/NioUtils.h
        common/NioUtils.cpp
        common/VirtualMachineEnv.h
        common/VirtualMachineEnv.cpp

        jni/string/jni_string.h
        jni/string/jni_string.cpp
        jni/embedded_file.h
        jni/jni_util.cc
        jni/jni_util.h
        jni/jni_weak_ref.cc
        jni/jni_weak_ref.h
        jni/scoped_java_ref.cc
        jni/scoped_java_ref.h

        jni/java_class.hpp
        jni/java_class.cpp
        jni/java_class_global_def.hpp
        jni/java_class_global_def.cpp
        jni/java_global_ref_by_copy.cpp
        jni/java_global_ref_by_copy.hpp
        jni/java_global_ref_by_move.hpp
        jni/java_global_ref_by_move.cpp
        jni/java_global_weak_ref.hpp
        jni/java_global_weak_ref.cpp
        jni/java_local_ref.hpp
        jni/java_method.hpp
        jni/java_method.cpp
        jni/jni_utils.hpp
        jni/jni_utils.cpp
        jni/util_android.h
        jni/util_android.cc
        jni/jobject_reference.h
        jni/jobject_reference.cc

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
        util/looper.h
        util/looper.cc
        util/optional.hpp
        util/scheduler.cpp
        util/scheduler.hpp
        util/scope_exit.hpp
        util/functional.hpp
        util/function_ref.hpp
        util/type_traits.hpp
        util/version_id.hpp

        variant/jni_variant_util.cc
        variant/jni_variant_util.h
        variant/variant.h
        variant/variant.cc
        variant/variant_util.h
        variant/variant_util.cc

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

        curl/curl_api.h
        curl/curl_api.cc
        curl/curl_header_parser.h
        curl/curl_header_parser.cc
        curl/curl_http_client.h
        curl/curl_http_client.cc
        curl/curl_http_request_handle.h
        curl/curl_http_request_handle.cc
        curl/curl_http_response.h
        curl/curl_http_response.cc
        curl/http_client.h
        curl/http_client_util.h
        curl/http_client_util.cc
        curl/scheduler.cc
        curl/scheduler.h
        curl/future.h
        curl/future.cc
        curl/interruptible_runner.h
        curl/interruptible_runner.cc
        curl/in_memory_request_response.h
        curl/in_memory_request_response.cc
        )

target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        flatbuffers
        curl::curl
        jsoncpp::jsoncpp
        android
        log
        openssl-ssl openssl-crypto
        nlohmann_json::nlohmann_json
        absl::core_headers
        absl::absl_check
        absl::absl_log
        absl::algorithm
        absl::base
        absl::bind_front
        absl::bits
        absl::btree
        absl::check
        absl::cleanup
        absl::cord
        absl::core_headers
        absl::die_if_null
        absl::debugging
        absl::dynamic_annotations
        absl::flags
        absl::flat_hash_map
        absl::flat_hash_set
        absl::function_ref
        absl::flags_commandlineflag
        absl::flags_marshalling
        absl::flags_parse
        absl::flags_reflection
        absl::flags_usage
        absl::hash
        absl::layout
        absl::log_initialize
        absl::log_severity
        absl::log
        absl::log_flags
        absl::log_globals
        absl::log_initialize
        absl::log_internal_message
        absl::memory
        absl::node_hash_map
        absl::node_hash_set
        absl::optional
        absl::span
        absl::status
        absl::statusor
        absl::strings
        absl::synchronization
        absl::time
        absl::type_traits
        absl::utility
        absl::variant
        absl::cord
        absl::random_random
        absl::raw_hash_set
        absl::hash
        absl::leak_check
        absl::memory
        absl::meta
        absl::stacktrace
        absl::status
        absl::statusor
        absl::str_format
        absl::strings
        absl::synchronization
        absl::time
        absl::any
        libuv
        )