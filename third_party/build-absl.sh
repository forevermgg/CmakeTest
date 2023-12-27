# abseil-cpp
#!/bin/bash
set -u
set -e
ANDROID_API=21

ANDROID_NDK_HOME=${1:-$ANDROID_NDK_HOME}
ANDROID_SDK_HOME=${2:-$ANDROID_HOME}

echo "ANDROID_NDK_HOME = $ANDROID_NDK_HOME"
echo "ANDROID_SDK_HOME = $ANDROID_SDK_HOME"

WORK_DIR=`pwd`
ABSL_DIR=`pwd`/abseil-cpp
echo "ABSL_DIR = $ABSL_DIR"

if [ -d $WORK_DIR/prefix/absl ]; then
    echo "Target folder exists. Remove $WORK_DIR/prefix/absl to rebuild"
    exit 0
fi

for ARCH in armeabi-v7a arm64-v8a x86 x86_64; do
# for ARCH in armeabi-v7a; do
    cd "$ABSL_DIR"
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
    $ANDROID_SDK_HOME/cmake/3.10.2.4988404/bin/cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_TOOLCHAIN_FILE=$ANDROID_SDK_HOME/ndk/20.0.5594570/build/cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DANDROID_ABI=$ARCH -DANDROID_PLATFORM=android-24 -DCMAKE_INSTALL_PREFIX=$WORK_DIR/prefix/absl/$ARCH/ -DBUILD_SHARED_LIBS=OFF -L  ..
    make && make install
    cd "$ABSL_DIR"
    # find ./ -name "*.o" | xargs ar cr libabsl.a
    find . -name "*.o" | xargs -n1 #打印一下所有.o。
    find . -name "*.o" | xargs ar -rcs libabsl.a #把所有.o生成.a
    ranlib libabsl.a
    cp $ABSL_DIR/libabsl.a $WORK_DIR/prefix/absl/$ARCH/
    rm -rf libabsl.a
    rm -rf $WORK_DIR/prefix/absl/$ARCH/lib
    cd "$WORK_DIR"
    cd ../
    echo "pwd = `pwd` $WORK_DIR"
done
### hava a bug 不能使用
