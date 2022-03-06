#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <AndroidLogDefine.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

void playerCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    
}

void initOpenSLES(){
    // 创建接口引擎对象
    SLObjectItf  engine_object = NULL;
    SLEngineItf engine_engine;
    slCreateEngine(&engine_object,0,NULL,0,NULL,NULL);

    (*engine_object)->Realize(engine_object, SL_BOOLEAN_FALSE);
    (*engine_object)->GetInterface(engine_object, SL_IID_ENGINE, &engine_engine);

    // 设置混音器
    static SLObjectItf output_mix_object = NULL;
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*engine_engine)->CreateOutputMix(engine_engine, &output_mix_object, 1, ids, req);
    (*output_mix_object)->Realize(output_mix_object, SL_BOOLEAN_FALSE);
    SLEnvironmentalReverbItf output_mix_environmental_reverb = NULL;
    (*output_mix_object)->GetInterface(output_mix_object, SL_IID_ENVIRONMENTALREVERB,
                                       &output_mix_environmental_reverb);
    SLEnvironmentalReverbSettings reverb_settings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    (*output_mix_environmental_reverb)->SetEnvironmentalReverbProperties(output_mix_environmental_reverb,
                                                                         &reverb_settings);

    // 创建播放器
    SLObjectItf pPlayer = NULL;
    SLPlayItf pPlayItf = NULL;
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&simpleBufferQueue, &formatPcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, output_mix_object};
    SLDataSink audioSnk = {&outputMix, NULL};
    SLInterfaceID interfaceIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE};
    SLboolean interfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*engine_engine)->CreateAudioPlayer(engine_engine, &pPlayer, &audioSrc, &audioSnk, 3,
                                       interfaceIds, interfaceRequired);
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &pPlayItf);
    // 3.4 设置缓存队列和回调函数
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playerCallback, NULL);
    // 3.5 设置播放状态
    (*pPlayItf)->SetPlayState(pPlayItf, SL_PLAYSTATE_PLAYING);
    // 3.6 调用回调函数
    //playerCallback(playerBufferQueue, NULL);
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

    initOpenSLES();

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