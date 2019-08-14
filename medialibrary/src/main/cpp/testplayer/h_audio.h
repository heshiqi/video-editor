//
// Created by 何士奇 on 2019-08-08.
//

#pragma once
#ifndef VIDEO_EDITOR_H_AUDIO_H
#define VIDEO_EDITOR_H_AUDIO_H

#include "h_base_player.h"
#include "h_queue.h"
#include "h_java_call.h"
#include "h_stataus.h"
#include "android_log.h"
extern "C"{
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};
class HAudio : public HBasePlayer {
public:
    HQueue *queue = NULL;
    HPlayStatus *mPlayStatus = NULL;
    JavaCall *mJavaCall = NULL;
    pthread_t audioThread;

    int ret = 0;//函数调用返回结果
    int64_t dst_layout = 0;//重采样为立体声
    int dst_nb_samples = 0;// 计算转换后的sample个数 a * b / c
    int nb = 0;//转换，返回值为转换后的sample个数
    uint8_t *out_buffer = NULL;//buffer 内存区域
    int out_channels = 0;//输出声道数
    int data_size = 0;//buffer大小
    enum AVSampleFormat dst_format;
    //opensl es

    void *buffer = NULL;
    int pcmsize = 0;
    int sample_rate = 44100;
    bool isExit = false;
    bool isVideo = false;

    bool isReadPacketFinish = true;
    AVPacket *packet;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmPlayerVolume = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

public:

    HAudio(HPlayStatus *playStatus, JavaCall *javaCall);
    ~HAudio();
    void setVideo(bool video);
    void playAudio();
    int getPcmData(void **pcm);
    int initOpenSL();
    void pause();
    void resume();
    void realease();
    sl_uint32_t getSLSampleRate();
    void setClock(int secds);
};

#endif //VIDEO_EDITOR_H_AUDIO_H
