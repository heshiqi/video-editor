//
// Created by 何士奇 on 2019-08-08.
//

#ifndef VIDEO_EDITOR_H_VIDEO_H
#define VIDEO_EDITOR_H_VIDEO_H

#include "h_base_player.h"
#include "h_queue.h"
#include "h_java_call.h"
#include "android_log.h"
#include "h_audio.h"

extern "C"
{
#include <libavutil/time.h>
};

class HVideo : public HBasePlayer {
public:
    HQueue *queue = NULL;
    HAudio *hAudio = NULL;
    HPlayStatus *hPlayStatus = NULL;
    pthread_t videoThread;
    pthread_t decFrame;
    JavaCall *hjavaCall = NULL;

    double delayTime = 0;
    int rate = 0;
    bool isExit = true;
    bool isExit2 = true;
    int codecType = -1;
    double video_clock = 0;
    double framePts = 0;
    bool frameratebig = false;
    int playcount = -1;

public:
    HVideo(JavaCall *javaCall, HAudio *audio, HPlayStatus *playStatus);

    ~HVideo();

    void playVideo(int codecType);

    void decodVideo();

    void release();

    double synchronize(AVFrame *srcFrame, double pts);

    double getDelayTime(double diff);

    void setClock(int secds);
};

#endif //VIDEO_EDITOR_H_VIDEO_H
