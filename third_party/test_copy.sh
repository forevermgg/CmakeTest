#!/bin/bash
# https://lvv.me/posts/2019/02/16-build_boringssl_for_ios/
set -u
set -e

export BASE=$(realpath ${BASE:-$(pwd)})

export PREFIX=$BASE/prefix/boringssl

function build_BoringSSL_Cp {
  ABI=$1
  BUILD_ARCHS=$1

}

build_BoringSSL_Cp armeabi-v7a
build_BoringSSL_Cp arm64-v8a
build_BoringSSL_Cp x86
build_BoringSSL_Cp x86_64