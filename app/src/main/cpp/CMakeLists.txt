cmake_minimum_required(VERSION 3.4.1)

find_library(log-lib log)
find_library(android-lib android)
find_library(oepn-sles OpenSLES)

FILE(GLOB SRC_C ${CMAKE_SOURCE_DIR}/*.cpp)
add_library(native-lib
            SHARED
            ${SRC_C})

set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/../jniLibs)
#导入FFmpeg的库

add_library(libavutil SHARED IMPORTED)
set_target_properties(libavutil
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavutil.so)

add_library(libavcodec SHARED IMPORTED)
set_target_properties(libavcodec
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavcodec.so)

add_library(libavformat SHARED IMPORTED)
set_target_properties(libavformat
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavformat.so)

add_library(libavfilter SHARED IMPORTED)
set_target_properties(libavfilter
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavfilter.so)

add_library(libswscale SHARED IMPORTED)
set_target_properties(libswscale
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswscale.so)

add_library(libswresample SHARED IMPORTED)
set_target_properties(libswresample
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswresample.so)

add_library(libavdevice SHARED IMPORTED)
set_target_properties(libavdevice
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavdevice.so)

add_library(libpostproc SHARED IMPORTED)
set_target_properties(libpostproc
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libpostproc.so)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${JNI_LIBS_DIR}/include)

target_link_libraries(
        native-lib
        ${log-lib}
        ${android-lib}
        ${oepn-sles}
        libavutil
        libavcodec
        libavformat
        libavfilter
        libswscale
        libswresample
        libavdevice
        libpostproc
        )
