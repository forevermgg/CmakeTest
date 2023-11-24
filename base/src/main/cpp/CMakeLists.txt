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
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
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
        )