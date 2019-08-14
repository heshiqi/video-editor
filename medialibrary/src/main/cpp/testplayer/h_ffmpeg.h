//
// Created by 何士奇 on 2019-08-08.
//

#ifndef VIDEO_EDITOR_H_FFMPEG_H
#define VIDEO_EDITOR_H_FFMPEG_H

#include "android_log.h"
#include "pthread.h"
#include "h_base_player.h"
#include "h_java_call.h"
#include "h_audio.h"
#include "h_video.h"
#include "h_play_status.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavformat/avio.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

class HFFmpeg {
public:
    const char *urlpath = NULL;
    JavaCall *mJavaCall = NULL;
    pthread_t decodThread;
    AVFormatContext *in_format_context = NULL;//封装格式上下文
    int duration = 0;
    HAudio *mAudio = NULL;
    HVideo *mVideo = NULL;
    HPlayStatus *mPlayStatus = NULL;
    bool exit = false;
    bool exitByUser = false;
    int mimeType = 1;
    bool isavi = false;
    bool isOnlyMusic = false;

    pthread_mutex_t init_mutex;
    pthread_mutex_t seek_mutex;
public:
    HFFmpeg(JavaCall *javaCall, const char *urlpath, bool onlymusic);

    ~HFFmpeg();

    int preparedFFmpeg();

    int decodeFFmpeg();

    int start();

    int seek(int64_t sec);

    int getDuration();

    void release();

    void pause();

    void resume();

    int getMimeType(const char *codecName);

    int getVideoWidth();

    int getVideoHeight();
};

#endif //VIDEO_EDITOR_H_FFMPEG_H
