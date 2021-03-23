#!/bin/bash

API=21
#arm64  x86 x86_64 对应 aarch64  i686  x86_64 
ARCH=arm64
ARCH2=aarch64    

PREFIX=$(pwd)/android-so/$ARCH

TOOLCHAIN=/Users/z/Library/Android/sdk/ndk/android-ndk-r21e/toolchains/llvm/prebuilt/darwin-x86_64

build()
{
./configure \
--prefix=$PREFIX \
--disable-static \
--enable-shared \
--enable-small \
--enable-gpl \
--disable-postproc \
--disable-doc \
--disable-programs \
--disable-avdevice \
--enable-cross-compile \
--target-os=android \
--arch=$ARCH \
--cc=$TOOLCHAIN/bin/$ARCH2-linux-android$API-clang \
--cross-prefix=$TOOLCHAIN/bin/$ARCH2-linux-android- 

make clean
make -j4
make install
}

build