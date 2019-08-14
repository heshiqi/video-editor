//
// Created by 何士奇 on 2019-08-08.
//

#ifndef VIDEO_EDITOR_H_QUEUE_H
#define VIDEO_EDITOR_H_QUEUE_H

#include "queue"
#include "h_play_status.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class HQueue {
public:
    std::queue<AVPacket *> queuePacket;
    std::queue<AVFrame *> queueFrame;
    pthread_mutex_t mutexFrame;
    pthread_cond_t condFrame;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    HPlayStatus *hPlayStatus = NULL;

public:
    HQueue(HPlayStatus *playStatus);

    ~HQueue();

    int putAvpacket(AVPacket *avPacket);

    int getAvpacket(AVPacket *avPacket);

    int clearAvpacket();

    int clearToKeyFrame();

    int putAvframe(AVFrame *avFrame);

    int getAvframe(AVFrame *avFrame);

    int clearAvFrame();

    void release();

    int getAvPacketSize();

    int getAvFrameSize();

    int noticeThread();
};

#endif //VIDEO_EDITOR_H_QUEUE_H
