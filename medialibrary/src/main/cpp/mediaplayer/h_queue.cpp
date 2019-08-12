//
// Created by 何士奇 on 2019-08-08.
//

#include "h_queue.h"
#include "../android_log.h"

HQueue::HQueue(HPlayStatus *playStatus) {
    hPlayStatus = playStatus;
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);
    pthread_mutex_init(&mutexFrame, NULL);
    pthread_cond_init(&condFrame, NULL);
}

HQueue::~HQueue() {
    hPlayStatus = NULL;
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
    pthread_mutex_destroy(&mutexFrame);
    pthread_cond_destroy(&condFrame);
    LOGD("HQueue 释放~~~~~~~~~~~~~~~~~");
}

void HQueue::release() {
    noticeThread();
    clearAvpacket();
    clearAvFrame();
    LOGD("HQueue %s", __FUNCTION__);
}

int HQueue::putAvpacket(AVPacket *avPacket) {

    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(avPacket);
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int HQueue::getAvpacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    while (hPlayStatus != NULL && !hPlayStatus->exit) {
        if (queuePacket.size() > 0) {
            AVPacket *pkt = queuePacket.front();
            if (av_packet_ref(avPacket, pkt) == 0) {
                queuePacket.pop();
            }
            av_packet_free(&pkt);
            av_free(pkt);
            pkt = NULL;
            break;
        } else {
            if (!hPlayStatus->exit) {
                pthread_cond_wait(&condPacket, &mutexPacket);
            }
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int HQueue::clearAvpacket() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()) {
        AVPacket *pkt = queuePacket.front();
        queuePacket.pop();
        av_free(pkt->data);
        av_free(pkt->buf);
        av_free(pkt->side_data);
        pkt = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int HQueue::getAvPacketSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

int HQueue::putAvframe(AVFrame *avFrame) {
    pthread_mutex_lock(&mutexFrame);
    queueFrame.push(avFrame);
    pthread_cond_signal(&condFrame);
    pthread_mutex_unlock(&mutexFrame);
    return 0;
}

int HQueue::getAvframe(AVFrame *avFrame) {
    pthread_mutex_lock(&mutexFrame);

    while (hPlayStatus != NULL && !hPlayStatus->exit) {
        if (queueFrame.size() > 0) {
            AVFrame *frame = queueFrame.front();
            if (av_frame_ref(avFrame, frame) == 0) {
                queueFrame.pop();
            }
            av_frame_free(&frame);
            av_free(frame);
            frame = NULL;
            break;
        } else {
            if (!hPlayStatus->exit) {
                pthread_cond_wait(&condFrame, &mutexFrame);
            }
        }
    }
    pthread_mutex_unlock(&mutexFrame);
    return 0;
}

int HQueue::clearAvFrame() {
    pthread_cond_signal(&condFrame);
    pthread_mutex_lock(&mutexFrame);
    while (!queueFrame.empty()) {
        AVFrame *frame = queueFrame.front();
        queueFrame.pop();
        av_frame_free(&frame);
        av_free(frame);
        frame = NULL;
    }
    pthread_mutex_unlock(&mutexFrame);
    return 0;
}

int HQueue::getAvFrameSize() {
    int size = 0;
    pthread_mutex_lock(&mutexFrame);
    size = queueFrame.size();
    pthread_mutex_unlock(&mutexFrame);
    return size;
}

int HQueue::noticeThread() {
    pthread_cond_signal(&condFrame);
    pthread_cond_signal(&condPacket);
    return 0;
}

int HQueue::clearToKeyFrame() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()) {
        AVPacket *pkt = queuePacket.front();
        if (pkt->flags != AV_PKT_FLAG_KEY) {
            queuePacket.pop();
            av_free(pkt->data);
            av_free(pkt->buf);
            av_free(pkt->side_data);
            pkt = NULL;
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}