#!/bin/bash

API=19
#armv7-a
ARCH=armv7 

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
--disable-doc \
--disable-programs \
--disable-avdevice \
--enable-cross-compile \
--target-os=android \
--arch=$ARCH \
--cc=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- 

make clean
make -j4
make install
}

build