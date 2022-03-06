//
// Created by z on 3/6/22.
//

#ifndef FFMPEG_ANDROIDLOGDEFINE_H
#define FFMPEG_ANDROIDLOGDEFINE_H

#include <Android/log.h>

#define TAG "JNI_FFMPEG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#endif //FFMPEG_ANDROIDLOGDEFINE_H
