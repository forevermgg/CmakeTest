#!/bin/bash
set -u
set -e
ANDROID_API=21

ANDROID_NDK_HOME=${1:-$ANDROID_NDK_HOME}
ANDROID_SDK_HOME=${2:-$ANDROID_HOME}

echo "ANDROID_NDK_HOME = $ANDROID_NDK_HOME"
echo "ANDROID_SDK_HOME = $ANDROID_SDK_HOME"

WORK_DIR=`pwd`
LIBUV_DIR=`pwd`/libuv
echo "LIBUV_DIR = $LIBUV_DIR"

if [ -d $WORK_DIR/prefix/libuv ]; then
    echo "Target folder exists. Remove $WORK_DIR/prefix/libuv to rebuild"
    exit 0
fi

for ARCH in armeabi-v7a arm64-v8a x86 x86_64; do
    cd "$LIBUV_DIR"
    build_directory="$PWD/build"
    if [ -d "build_directory" ]; then
      echo "build_directory $build_directory exists."
      cd build
    else
      echo "build_directory $build_directory does not exist."
      mkdir -p build && cd build
    fi

    # 检查文件夹是否存在
    if [ -d "$build_directory" ]; then
      echo "Directory $build_directory exists."
    else
      echo "Directory $build_directory does not exist."
    fi
    $ANDROID_SDK_HOME/cmake/3.10.2.4988404/bin/cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_SDK_HOME/ndk/20.0.5594570/build/cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DANDROID_ABI=$ARCH -DANDROID_PLATFORM=android-24 ..
    cd ../
    $ANDROID_SDK_HOME/cmake/3.10.2.4988404/bin/cmake --build build
    ls -lh build
    echo "pwd = `pwd` $WORK_DIR"
    mkdir -p $WORK_DIR/prefix/libuv/$ARCH/lib
    cp build/libuv.a $WORK_DIR/prefix/libuv/$ARCH/lib
    cp build/libuv.so $WORK_DIR/prefix/libuv/$ARCH/lib
    cp -r include/ $WORK_DIR/prefix/libuv/$ARCH/include/
    rm -rf build
done