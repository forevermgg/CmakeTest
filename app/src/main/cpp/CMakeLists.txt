# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.19)

# Declares and names the project.
project("app")

include(../cmake/Common.cmake)

set(EXTERNAL_CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../cmake/external;${EXTERNAL_CMAKE_SOURCE_DIR}")
message("EXTERNAL_CMAKE_SOURCE_DIR:${EXTERNAL_CMAKE_SOURCE_DIR}")

include(../../../../cmake/GetGitVersion.cmake)
include(../cmake/EnableWarnings.cmake)
include(../../../../cmake/Version.cmake)
include(../../../../cmake/Test.cmake)
# include(../cmake/external/googletest.cmake)
# include(../cmake/external/spdlog.cmake)
# include(../cmake/external/nlohmann_json.cmake)
# include(../cmake/FindLibUV.cmake)
# include(../cmake/FindOpenSSL.cmake)

message("APP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Read the git tags to determine the project version
# include(GetGitVersion)
get_git_version(GIT_VERSION)
# Tell the user what versions we are using
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION ${GIT_VERSION})
message("-- Version: ${VERSION}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -O3 -flto")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Oz")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz")

string(TIMESTAMP COMPILE_TIME %m%d_%H%M)

find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_VERBOSE_MAKEFILEON OFF)
set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS} -ggdb -Og -fPIC -w -Wl,--export-dynamic -Wno-deprecated -fpermissive -save-temps")


message("GIT_COMMIT: ${GIT_COMMIT}")
add_definitions(-DGIT_COMMIT_SHA1="${GIT_COMMIT}")

execute_process(COMMAND bash "-c" "git log|head -n 1|awk '{printf $2}'"
        OUTPUT_VARIABLE GIT_COMMIT_ID
        ERROR_QUIET)

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_COMMIT_ID}" STREQUAL "")
    set(GIT_COMMIT_ID "N/A")
    set(GIT_TAG "N/A")
    set(GIT_BRANCH "N/A")
else()
    execute_process(
            COMMAND git describe --exact-match --tags
            OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE GIT_BRANCH)

    string(STRIP "${GIT_COMMIT_ID}" GIT_COMMIT_ID)
    string(STRIP "${GIT_COMMIT_ID}" GIT_COMMIT_ID)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif()

set(VERSION "const char* GIT_COMMIT_ID=\"${GIT_COMMIT_ID}\";
const char* GIT_TAG=\"${GIT_TAG}\";
const char* GIT_BRANCH=\"${GIT_BRANCH}\";")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.h VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.h "${VERSION}")
endif()

ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.h
        ${CMAKE_CURRENT_BINARY_DIR}/_version.h
        COMMAND ${CMAKE_COMMAND} -P
        ../../../../cmake/Version.cmake)


macro(get_git_hash GIT_COMMIT_ID)   # 宏的开始
    find_package(Git QUIET)     # 查找Git，QUIET静默方式不报错
    if(GIT_FOUND)
        execute_process(COMMAND bash "-c" "git log|head -n 1|awk '{printf $2}'"
                OUTPUT_VARIABLE ${GIT_COMMIT_ID}
                ERROR_QUIET)

        execute_process(
                COMMAND git rev-parse --abbrev-ref HEAD
                OUTPUT_VARIABLE ${GIT_BRANCH})
    endif()
endmacro()                      # 宏的结束

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/git_version.h.in  # 输入
        ${CMAKE_CURRENT_SOURCE_DIR}/git_version.h            # 输出
        @ONLY     # 只接受形如@VAR@的占位符
)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        app

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
# log https://www.cnblogs.com/fnlingnzb-learner/p/7221648.html

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        app
        # spdlog::spdlog
        # LibUV::LibUV
        # nlohmann_json::nlohmann_json
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})