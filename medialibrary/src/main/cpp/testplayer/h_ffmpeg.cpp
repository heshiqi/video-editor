//
// Created by 何士奇 on 2019-08-08.
//

#include "h_ffmpeg.h"
#include "h_stataus.h"

extern "C" {
#include "ffmpeg_utils.h"
}

void *decodeThread(void *data) {
    HFFmpeg *hfFmpeg = (HFFmpeg *) data;
    hfFmpeg->decodeFFmpeg();
    pthread_exit(&hfFmpeg->decodThread);
}


int HFFmpeg::preparedFFmpeg() {
    pthread_create(&decodThread, NULL, decodeThread, this);
    return 0;
}

HFFmpeg::HFFmpeg(JavaCall *javaCall, const char *url, bool onlymusic) {
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
    exitByUser = false;
    isOnlyMusic = onlymusic;
    mJavaCall = javaCall;
    urlpath = url;
    mPlayStatus = new HPlayStatus();
}

HFFmpeg::~HFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
    LOGE("~WlFFmpeg() 释放了");
}

int avformat_interrupt_cb(void *ctx) {
    HFFmpeg *hfFmpeg = (HFFmpeg *) ctx;
    if (hfFmpeg->mPlayStatus->exit) {
        LOGE("avformat_interrupt_cb return 1");
        return AVERROR_EOF;
    }
    LOGE("avformat_interrupt_cb return 0");
    return 0;
}

int HFFmpeg::decodeFFmpeg() {
    pthread_mutex_lock(&init_mutex);
    exit = false;
    isavi = false;
    av_register_all();
    avformat_network_init();
    if (open_input_file(urlpath, &in_format_context)) {
        if (mJavaCall != NULL) {
            mJavaCall->onError(WL_THREAD_CHILD, WL_FFMPEG_CAN_NOT_OPEN_URL, "不能打开文件");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    in_format_context->interrupt_callback.callback = avformat_interrupt_cb;
    in_format_context->interrupt_callback.opaque = this;
    duration = in_format_context->duration / 1000000;

    int video_index, audio_index;

    //初始化音频信息
    mAudio = new HAudio(mPlayStatus, mJavaCall);
    if (init_input_codec(&in_format_context, &(mAudio->avCodecContext), &audio_index,
                         AVMEDIA_TYPE_AUDIO)) {
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    mAudio->time_base = in_format_context->streams[audio_index]->time_base;
    mAudio->streamIndex = audio_index;
    mAudio->duration = in_format_context->duration / 1000000;
    mAudio->sample_rate = mAudio->avCodecContext->sample_rate;

    //初始化视频信息
    if (!isOnlyMusic) {
        mVideo = new HVideo(mJavaCall, mAudio, mPlayStatus);
        if (init_input_codec(&in_format_context, &(mVideo->avCodecContext), &video_index,
                             AVMEDIA_TYPE_VIDEO)) {
            exit = true;
            pthread_mutex_unlock(&init_mutex);
            return -1;
        }
        int num = in_format_context->streams[video_index]->avg_frame_rate.num;
        int den = in_format_context->streams[video_index]->avg_frame_rate.den;
        if (num != 0 && den != 0) {
            int fps = num / den;
            mVideo->streamIndex = video_index;
            mVideo->time_base = in_format_context->streams[video_index]->time_base;
            mVideo->rate = 1000 / fps;
            if (fps >= 60) {
                mVideo->frameratebig = true;
            } else {
                mVideo->frameratebig = false;
            }
        }
        mAudio->setVideo(true);
        LOGE("codec name is %s", mVideo->avCodecContext->codec->name);
        LOGE("codec long name is %s", mVideo->avCodecContext->codec->long_name);
        if (!mJavaCall->isOnlySoft(WL_THREAD_CHILD)) {
            mimeType = getMimeType(mVideo->avCodecContext->codec->name);
        } else {
            mimeType = -1;
        }
        if (mimeType != -1) {
            mJavaCall->onInitMediacodec(WL_THREAD_CHILD, mimeType,
                                        mVideo->avCodecContext->width,
                                        mVideo->avCodecContext->height,
                                        mVideo->avCodecContext->extradata_size,
                                        mVideo->avCodecContext->extradata_size,
                                        mVideo->avCodecContext->extradata,
                                        mVideo->avCodecContext->extradata);
        }
        mVideo->duration = in_format_context->duration / 1000000;
    }
    mJavaCall->onParpared(WL_THREAD_CHILD);
    exit = true;
    pthread_mutex_unlock(&init_mutex);
    return 0;
}

int HFFmpeg::getDuration() {
    return duration;
}

void init_bitstreamfiltercontext(AVBitStreamFilterContext **stream_filter_context, int mimeType) {
    AVBitStreamFilterContext *filter_context = NULL;
    if (mimeType == 1) {
        filter_context = av_bitstream_filter_init("h264_mp4toannexb");
    } else if (mimeType == 2) {
        filter_context = av_bitstream_filter_init("hevc_mp4toannexb");
    } else if (mimeType == 3) {
        filter_context = av_bitstream_filter_init("h264_mp4toannexb");
    } else if (mimeType == 4) {
        filter_context = av_bitstream_filter_init("h264_mp4toannexb");
    }
    *stream_filter_context = filter_context;
}

int HFFmpeg::start() {
    exit = false;
    int ret = -1;
    if (mAudio != NULL) {
        mAudio->playAudio();
    }
    if (mVideo != NULL) {
        if (mimeType == -1) {
            mVideo->playVideo(0);
        } else {
            mVideo->playVideo(1);
        }
    }

    AVBitStreamFilterContext *mimType = NULL;
    init_bitstreamfiltercontext(&mimType, mimeType);

    while (!mPlayStatus->exit) {
        exit = false;
        if (mPlayStatus->pause)//暂停
        {
            av_usleep(1000 * 100);
            continue;
        }
        if (mAudio != NULL && mAudio->queue->getAvPacketSize() > 100) {
            LOGE("audio 等待..........");
            av_usleep(1000 * 100);
            continue;
        }
        if (mVideo != NULL && mVideo->queue->getAvPacketSize() > 100) {
            LOGE("video 等待..........");
            av_usleep(1000 * 100);
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        ret = av_read_frame(in_format_context, packet);
        pthread_mutex_unlock(&seek_mutex);
        if (mPlayStatus->seek) {
            av_packet_free(&packet);
            av_free(packet);
            continue;
        }
        if (ret == 0) {
            if (mAudio != NULL && packet->stream_index == mAudio->streamIndex) {
                mAudio->queue->putAvpacket(packet);
            } else if (mVideo != NULL && packet->stream_index == mVideo->streamIndex) {
                if (mimType != NULL && !isavi) {
                    LOGD("转换 转化 转化 转换 转换");
                    uint8_t *data;
                    av_bitstream_filter_filter(mimType,
                                               in_format_context->streams[mVideo->streamIndex]->codec,
                                               NULL, &data, &packet->size, packet->data,
                                               packet->size, 0);
                    uint8_t *tdata = NULL;
                    tdata = packet->data;
                    packet->data = data;
                    if (tdata != NULL) {
                        av_free(tdata);
                    }
                }
                mVideo->queue->putAvpacket(packet);
            } else {
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;
            }
        } else {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            if ((mVideo != NULL && mVideo->queue->getAvFrameSize() == 0) ||
                (mAudio != NULL && mAudio->queue->getAvPacketSize() == 0)) {
                mPlayStatus->exit = true;
                break;
            }
        }
    }
    if (mimType != NULL) {
        av_bitstream_filter_close(mimType);
    }
    if (!exitByUser && mJavaCall != NULL) {
        mJavaCall->onComplete(WL_THREAD_CHILD);
    }
    exit = true;
    return 0;
}

void HFFmpeg::release() {
    mPlayStatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    LOGE("开始释放 wlffmpeg");
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000)//十秒钟还没有退出就自动强制退出
        {
            exit = true;
        }
        LOGE("wait ffmpeg  exit %d", sleepCount);
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }
    if (mAudio != NULL) {
        mAudio->realease();
        delete (mAudio);
        mAudio = NULL;
        LOGE("释放audio....................................");
    }
    if (mVideo != NULL) {
        mVideo->release();
        delete (mVideo);
        mVideo = NULL;
        LOGE("释放video....................................");
    }
    if (in_format_context != NULL) {
        avformat_close_input(&in_format_context);
        avformat_free_context(in_format_context);
        in_format_context = NULL;
        LOGE("释放format...................................");
    }
    if (mJavaCall != NULL) {
        mJavaCall = NULL;
    }
    if(mPlayStatus!=NULL){
        delete mPlayStatus;
    }
    pthread_mutex_unlock(&init_mutex);
}

void HFFmpeg::pause() {
    if (mPlayStatus != NULL) {
        mPlayStatus->pause = true;
        if (mAudio != NULL) {
            mAudio->pause();
        }
    }
}

void HFFmpeg::resume() {
    if (mPlayStatus != NULL) {
        mPlayStatus->pause = false;
        if (mAudio != NULL) {
            mAudio->resume();
        }
    }
}

int HFFmpeg::getMimeType(const char *codecName) {

    if (strcmp(codecName, "h264") == 0) {
        return 1;
    }
    if (strcmp(codecName, "hevc") == 0) {
        return 2;
    }
    if (strcmp(codecName, "mpeg4") == 0) {
        isavi = true;
        return 3;
    }
    if (strcmp(codecName, "wmv3") == 0) {
        isavi = true;
        return 4;
    }

    return -1;
}

int HFFmpeg::seek(int64_t sec) {
    if (sec >= duration) {
        return -1;
    }
    if (mPlayStatus->load) {
        return -1;
    }
    if (in_format_context != NULL) {
        mPlayStatus->seek = true;
        pthread_mutex_lock(&seek_mutex);
        int64_t rel = sec * AV_TIME_BASE;
        int ret = avformat_seek_file(in_format_context, -1, INT64_MIN, rel, INT64_MAX, 0);
        if (mAudio != NULL) {
            mAudio->queue->clearAvpacket();
//            av_seek_frame(in_format_context, wlAudio->streamIndex, sec * wlAudio->time_base.den, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
            mAudio->setClock(0);
        }
        if (mVideo != NULL) {
            mVideo->queue->clearAvFrame();
            mVideo->queue->clearAvpacket();
//            av_seek_frame(in_format_context, wlVideo->streamIndex, sec * wlVideo->time_base.den, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
            mVideo->setClock(0);
        }
        mAudio->clock = 0;
        mAudio->now_time = 0;
        pthread_mutex_unlock(&seek_mutex);
        mPlayStatus->seek = false;
    }
    return 0;
}

int HFFmpeg::getVideoWidth() {
    if (mVideo != NULL && mVideo->avCodecContext != NULL) {
        return mVideo->avCodecContext->width;
    }
    return 0;
}

int HFFmpeg::getVideoHeight() {
    if (mVideo != NULL && mVideo->avCodecContext != NULL) {
        return mVideo->avCodecContext->height;
    }
    return 0;
}