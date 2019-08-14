//
// Created by 何士奇 on 2019-08-08.
//

#ifndef VIDEO_EDITOR_H_BASE_PLAYER_H
#define VIDEO_EDITOR_H_BASE_PLAYER_H

extern "C" {
#include <libavcodec/avcodec.h>
};

class HBasePlayer {
public:
    int streamIndex;
    int duration;
    double clock = 0;
    double now_time = 0;
    AVCodecContext *avCodecContext = NULL;
    AVRational time_base;
public:
    HBasePlayer();

    ~HBasePlayer();
};

#endif //VIDEO_EDITOR_H_BASE_PLAYER_H
