#!/bin/bash
set -u
set -e
ANDROID_API=21

ANDROID_NDK_HOME=${1:-$ANDROID_NDK_HOME}
ANDROID_SDK_HOME=${2:-$ANDROID_HOME}

echo "ANDROID_NDK_HOME = $ANDROID_NDK_HOME"
echo "ANDROID_SDK_HOME = $ANDROID_SDK_HOME"

WORK_DIR=`pwd`
PROTOBUF_DIR=`pwd`/protobuf
echo "PROTOBUF_DIR = $PROTOBUF_DIR"

if [ -d $WORK_DIR/prefix/protobuf ]; then
    echo "Target folder exists. Remove $WORK_DIR/prefix/protobuf to rebuild"
    # exit 0
fi

for ARCH in armeabi-v7a arm64-v8a x86 x86_64; do
    cd "$PROTOBUF_DIR"
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
    $ANDROID_SDK_HOME/cmake/3.10.2.4988404/bin/cmake \
                                                    -Dprotobuf_BUILD_SHARED_LIBS=OFF \
                                                    -Dprotobuf_BUILD_STATIC_LIBS=ON \
                                                    -Dprotobuf_BUILD_TESTS=OFF \
                                                    -Dprotobuf_BUILD_TEST=OFF \
                                                    -Dprotobuf_BUILD_EXAMPLES=OFF \
                                                    -DCMAKE_VERBOSE_MAKEFILE=ON \
                                                    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_SDK_HOME/ndk/20.0.5594570/build/cmake/android.toolchain.cmake \
                                                    -DCMAKE_BUILD_TYPE=Debug \
                                                    -DCMAKE_INSTALL_PREFIX=$WORK_DIR/prefix/protobuf/$ARCH/lib \
                                                    -DANDROID_NDK=$ANDROID_NDK_HOME \
                                                    -DANDROID_TOOLCHAIN=clang \
                                                    -DANDROID_ABI=$ARCH \
                                                    -DANDROID_NATIVE_API_LEVEL=16 \
                                                    -DANDROID_STL=c++_shared \
                                                    -DANDROID_LINKER_FLAGS="-landroid -llog" \
                                                    -DANDROID_CPP_FEATURES="rtti exceptions" \
                                                    ..

    $ANDROID_SDK_HOME/cmake/3.10.2.4988404/bin/cmake --build . --target libprotobuf
    cp -p $PROTOBUF_DIR/build/libprotobuf.a $WORK_DIR/prefix/protobuf/$ARCH/lib/
done