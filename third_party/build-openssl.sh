#!/bin/bash
set -u
set -e

export OPENSSL_BRANCH=OpenSSL_1_1_1-stable
export OPENSSL_ANDROID_API=21

NDK=${1:-$NDK}

export ANDROID_NDK_HOME=$NDK

export BASE=$(realpath ${BASE:-$(pwd)})

export PREFIX=$BASE/prefix/openssl

if [ -d $PREFIX ]; then
    echo "Target folder exists. Remove $PREFIX to rebuild"
    exit 0
fi

mkdir -p $PREFIX

cd $BASE
if [ ! -d openssl ]; then
    git clone --depth 10 git://git.openssl.org/openssl.git --branch $OPENSSL_BRANCH
fi
cd openssl
echo "Building OpenSSL in $(realpath $PWD), deploying to $PREFIX"

export PATH=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH

function build_OpenSSL {
  ABI=$1
  BUILD_ARCHS=$1
  MINIMUM_API_LEVEL=$2

  echo "Building OpenSSL for ${ABI}"

  if [[ "$BUILD_ARCHS" = *"armeabi-v7a"* ]]; then
      ./Configure shared android-arm -D__ANDROID_API__=$OPENSSL_ANDROID_API --prefix=$PREFIX/$1
  fi

  if [[ "$BUILD_ARCHS" = *"arm64-v8a"* ]]; then
      ./Configure shared android-arm64 -D__ANDROID_API__=$OPENSSL_ANDROID_API --prefix=$PREFIX/$1
  fi

  if [[ "$BUILD_ARCHS" = *"x86"* ]]; then
      ./Configure shared android-x86 -D__ANDROID_API__=$OPENSSL_ANDROID_API --prefix=$PREFIX/$1
  fi

  if [[ "$BUILD_ARCHS" = *"x86_64"* ]]; then
      ./Configure shared android-x86_64 -D__ANDROID_API__=$OPENSSL_ANDROID_API --prefix=$PREFIX/$1
  fi
  make clean
  make depend
  make -j$(nproc) build_libs
  make -j$(nproc) install_sw
}

build_OpenSSL armeabi-v7a OPENSSL_ANDROID_API
build_OpenSSL arm64-v8a OPENSSL_ANDROID_API
build_OpenSSL x86 OPENSSL_ANDROID_API
build_OpenSSL x86_64 OPENSSL_ANDROID_API
