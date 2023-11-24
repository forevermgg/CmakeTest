#!/bin/bash
# https://lvv.me/posts/2019/02/16-build_boringssl_for_ios/
set -u
set -e

export BUILD_ARCHS=${BUILD_ARCHS:-arm_32 arm_64}
export BORINGSSL_BRANCH=fips-20230428
export BORINGSSL_ANDROID_API=21

NDK=${1:-$NDK}

export ANDROID_NDK_HOME=$NDK

export BASE=$(realpath ${BASE:-$(pwd)})

export PREFIX=$BASE/prefix/boringssl

if [ -d $PREFIX ]; then
    echo "Target folder exists. Remove $PREFIX to rebuild"
    # exit 1
fi

mkdir -p $PREFIX

cd $BASE
if [ ! -d boringssl ]; then
    git clone --depth 10 https://github.com/google/boringssl.git --branch $BORINGSSL_BRANCH
fi
cd boringssl
echo "Building boringssl in $(realpath $PWD), deploying to $PREFIX"

export PATH=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH

function merge_crypto_ssl_lib () {
  cd crypto && ar x libcrypto.a
  cd ../ssl && ar x libssl.a && cd ..
  ar r libboringssl.a ./crypto/*.o ./ssl/*.o
}

if [ -d "$PWD/build" ]; then
  echo "build_directory $PWD/build exists."
  rm -r $PWD/build
  exit 1
fi

echo "Building boringssl in $(realpath $PWD)"

if [[ "$BUILD_ARCHS" = *"arm_32"* ]]; then
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
    arch=armeabi-v7a
    mkdir ${arch} && cd ${arch}
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
          -DCMAKE_BUILD_TYPE=Debug \
          -DANDROID_ABI=armeabi-v7a \
          -DANDROID_NDK=$ANDROID_NDK_HOME \
          -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
          -DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCMAKE_SYSTEM_NAME=Android \
          -DANDROID_NATIVE_API_LEVEL=$BORINGSSL_ANDROID_API \
          -DCMAKE_INSTALL_PREFIX=$PREFIX/armeabi-v7a \
          -GNinja ../..
    ninja
    echo "Building boringssl armeabi-v7a"
    merge_crypto_ssl_lib
    cd ../../
fi


echo "Building boringssl in $(realpath $PWD)"
if [[ "$BUILD_ARCHS" = *"arm_64"* ]]; then
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
    arch=arm64-v8a
    mkdir ${arch} && cd ${arch}
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
          -DCMAKE_BUILD_TYPE=Debug \
          -DANDROID_ABI=arm64-v8a \
          -DANDROID_NDK=$ANDROID_NDK_HOME \
          -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
          -DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCMAKE_SYSTEM_NAME=Android \
          -DANDROID_NATIVE_API_LEVEL=$BORINGSSL_ANDROID_API \
          -DCMAKE_INSTALL_PREFIX=$PREFIX/arm64-v8a \
          -GNinja ../..
    ninja
    echo "Building boringssl arm64-v8a"
    merge_crypto_ssl_lib
    cd ../../
fi
