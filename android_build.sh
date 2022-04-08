# 用于编译 android 平台的脚本
#!/bin/bash
. /etc/profile

# android-ndk-r20b
# 定义几个变量
# armeabi-v7a armv7-a armv7a
# arm64-v8a armv8-a aarch64

ARCH=arm64
CPU=armv8-a
CPU_CLANG=aarch64
API=21
PREFIX=$(pwd)/android/$ARCH/$CPU
ANDROID_TOOLCHAINS_PATH=$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT=$ANDROID_TOOLCHAINS_PATH/sysroot
# 两个参数看文件路径决定
ANDROID_CC=$ANDROID_TOOLCHAINS_PATH/bin/$CPU_CLANG-linux-android$API-clang
CROSS_PREFIX=$ANDROID_TOOLCHAINS_PATH/bin/aarch64-linux-android-

build(){
	# 执行 .configure 文件
	./configure --prefix=${PREFIX} \
		--enable-gpl \
		--disable-static \
		--enable-shared \
		--enable-small \
		--disable-programs \
		--disable-ffmpeg \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-doc \
		--arch=$ARCH \
		--cpu=$CPU \
		--cross-prefix=${CROSS_PREFIX} \
		--enable-cross-compile \
		--sysroot=$SYSROOT \
		--target-os=android \
		--cc=$ANDROID_CC \
		--extra-cflags="-fpic" \

		# makefile 清除，就是执行了 makefile 文件里面的 clean 命令
	make clean
	# 运行 Makefile
	make -j8
	# 安装到指定 prefix 目录下
	make install
	# make clean
}



# 执行 build 函数
build

# 下面需要再编译其他的
# 需要重新设置参数
# CPU=NAME
# build build
