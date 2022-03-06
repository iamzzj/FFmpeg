#include <jni.h>
#include <string>
#include "AndroidLogDefine.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_iamzzj_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject j_object /* this */) {
    const char *hello = avcodec_configuration();

    return env->NewStringUTF(hello);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_iamzzj_ffmpeg_MainActivity_nPlay(JNIEnv *env, jobject thiz, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    av_register_all();

    // 播放网络流需要
    avformat_network_init();

    // 变量
    int ret;
    int best_audio_index;
    AVFormatContext *p_avformat_context = NULL;
    AVCodecParameters *p_avcodec_pars = NULL;
    AVCodec *p_avcodec = NULL;
    AVCodecContext *p_avcodec_context = NULL;
    AVPacket *p_avpacket = NULL;
    AVFrame *p_avframe = NULL;

    // 解封装
    ret = avformat_open_input(&p_avformat_context, url, NULL, NULL);
    if (ret != 0) {
        LOGE("avformat open input fail: %s\n", av_err2str(ret));
        goto err;
    }

    ret = avformat_find_stream_info(p_avformat_context, NULL);
    if (ret < 0) {
        LOGE("avformat find stream info fail: %s\n", av_err2str(ret));
        goto err;
    }

    best_audio_index = av_find_best_stream(p_avformat_context, AVMEDIA_TYPE_AUDIO,
                                           -1, -1, NULL, 0);

    if (best_audio_index < 0) {
        LOGE("av find best stream fail: %s\n", av_err2str(best_audio_index));
        goto err;
    }

    // 查找解码
    p_avcodec_pars = p_avformat_context->streams[best_audio_index]->codecpar;
    p_avcodec = avcodec_find_decoder(p_avcodec_pars->codec_id);
    if (!p_avcodec) {
        LOGE("avcodec find decoder fail\n");
        goto err;
    }
    // 打开解码器
    p_avcodec_context = avcodec_alloc_context3(p_avcodec);
    if (!p_avcodec_context) {
        LOGE("avcodec alloc context\n");
        goto err;
    }
    ret = avcodec_parameters_to_context(p_avcodec_context, p_avcodec_pars);
    if (ret < 0) {
        LOGE("avcodec parameters to context fail: %s\n", av_err2str(ret));
        goto err;
    }
    ret = avcodec_open2(p_avcodec_context, p_avcodec, NULL);
    if (ret != 0) {
        LOGE("avcodec open: %s\n", av_err2str(ret));
        goto err;
    }

    p_avpacket = av_packet_alloc();
    p_avframe = av_frame_alloc();

    while (av_read_frame(p_avformat_context, p_avpacket) >= 0) {
        if (p_avpacket->stream_index == best_audio_index) {
            ret = avcodec_send_packet(p_avcodec_context, p_avpacket);
            if (ret == 0) {
                ret = avcodec_receive_frame(p_avcodec_context, p_avframe);
                if (ret == 0) {
                    int dataSize = av_samples_get_buffer_size(
                            NULL,
                            p_avframe->channels,
                            p_avframe->nb_samples,
                            p_avcodec_context->sample_fmt,
                            0
                    );

                    LOGI("avframe data size: %d\n", dataSize);

                } else {
                    LOGE("avcodec receive frame fail: %s\n", av_err2str(ret));
                }
            } else {
                LOGE("avcodec receive packet fail: %s\n", av_err2str(ret));
            }
        }

        av_packet_unref(p_avpacket);
        av_frame_unref(p_avframe);
    }

    av_packet_free(&p_avpacket);
    av_frame_free(&p_avframe);

    err:
    if (p_avcodec_context) {
        avcodec_close(p_avcodec_context);
        avcodec_free_context(&p_avcodec_context);
    }
    if (p_avformat_context) {
        avformat_close_input(&p_avformat_context);
        avformat_free_context(p_avformat_context);
    }
    avformat_network_deinit();

    env->ReleaseStringUTFChars(url_, url);
}