#!/bin/bash
# https://lvv.me/posts/2019/02/16-build_boringssl_for_ios/
set -u
set -e

export BORINGSSL_BRANCH=fips-20230428
export BORINGSSL_ANDROID_API=21

NDK=${1:-$NDK}

export ANDROID_NDK_HOME=$NDK

export BASE=$(realpath ${BASE:-$(pwd)})

export PREFIX=$BASE/prefix/boringssl

if [ -d $PREFIX ]; then
    echo "Target folder exists. Remove $PREFIX to rebuild"
    exit 0
fi

mkdir -p $PREFIX

cd $BASE
if [ ! -d boringssl ]; then
    git clone --depth 10 https://github.com/google/boringssl.git --branch $BORINGSSL_BRANCH
fi
cd boringssl
echo "Building boringssl in $(realpath $PWD), deploying to $PREFIX"

export PATH=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH

if [ -d "$PWD/build" ]; then
  echo "build_directory $PWD/build exists."
  exit 0
fi

echo "Building boringssl in $(realpath $PWD)"

function build_BoringSSL {
  ABI=$1
  BUILD_ARCHS=$1
  MINIMUM_API_LEVEL=$2

  echo "Building OpenSSL for ${ABI}"

  echo "Building boringssl in $(realpath $PWD)"

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
  mkdir ${BUILD_ARCHS} && cd ${BUILD_ARCHS}
  cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DANDROID_ABI=$1 \
        -DANDROID_NDK=$ANDROID_NDK_HOME \
        -DCMAKE_ANDROID_ARCH_ABI=$1 \
        -DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_SYSTEM_NAME=Android \
        -DANDROID_NATIVE_API_LEVEL=$BORINGSSL_ANDROID_API \
        -DCMAKE_INSTALL_PREFIX=$PREFIX/$1 \
        -GNinja ../..
  ninja
  cd ../../

  mkdir -p $PREFIX/$BUILD_ARCHS/lib/
  mkdir -p $PREFIX/$BUILD_ARCHS/include/
  cp -p $BASE/boringssl/build/$BUILD_ARCHS/crypto/libcrypto.a $PREFIX/$BUILD_ARCHS/lib/
  cp -p $BASE/boringssl/build/$BUILD_ARCHS/decrepit/libdecrepit.a $PREFIX/$BUILD_ARCHS/lib/
  cp -p $BASE/boringssl/build/$BUILD_ARCHS/ssl/libssl.a $PREFIX/$BUILD_ARCHS/lib/
  cp -r $BASE/boringssl/include/ $PREFIX/$BUILD_ARCHS/include/
}

build_BoringSSL armeabi-v7a BORINGSSL_ANDROID_API
build_BoringSSL arm64-v8a BORINGSSL_ANDROID_API
build_BoringSSL x86 BORINGSSL_ANDROID_API
build_BoringSSL x86_64 BORINGSSL_ANDROID_API
