# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
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


set(JSON_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/json/json)
message("JSON_ROOT_DIR: ${JSON_ROOT_DIR}")
add_subdirectory(${JSON_ROOT_DIR}  ${JSON_ROOT_DIR}/build)
include_directories(${JSON_ROOT_DIR}/json/single_include)

set(ABSEIL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/abseil-cpp)
message("ABSEIL_ROOT_DIR: ${ABSEIL_ROOT_DIR}")
add_subdirectory(${ABSEIL_ROOT_DIR}  ${ABSEIL_ROOT_DIR}/build)
#include_directories(${ABSEIL_ROOT_DIR}/json/single_include)

add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        jni/string/jni_string.h
        jni/string/jni_string.cpp

        json/same.h
        json/same.cpp
        json/index.h
        json/type_traits.h
        json/value_as.h
        json/value_as.cc
        base.cpp)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
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
        )