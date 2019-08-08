//
// Created by 何士奇 on 2019-08-07.
//
#include "entity_factory.h"
#include "../videoeditor/android_log.h"

int init_av_codec_context_with_type(AVFormatContext *format_context, AVCodecContext **codec_context,AVCodec **codec,
                                    AVFrame *frame,
                                    void (*Callback)(AVFormatContext *, AVStream *, AVFrame *)) {
    AVCodecContext *context = NULL;
    AVCodec *avCodec;
    // 构建一个新stream
    AVStream *pAVStream = avformat_new_stream(format_context, 0);
    if (pAVStream == NULL) {
        return AVERROR(ENOMEM);
    }
    Callback(format_context, pAVStream, frame);
    avCodec = avcodec_find_encoder(pAVStream->codecpar->codec_id);
    if (!avCodec) {
        LOGE("Could not find encoder\n");
        return AVERROR(ENOMEM);
    }

    context = avcodec_alloc_context3(avCodec);
    if (!context) {
        LOGE("Could not allocate video codec context\n");
        return AVERROR(ENOMEM);
    }

    if ((avcodec_parameters_to_context(context, pAVStream->codecpar)) < 0) {
        LOGE("Failed to copy %s codec parameters to decoder context\n",
             av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        avcodec_free_context(&context);
        return AVERROR(ENOMEM);;
    }
    *codec=avCodec;
    *codec_context = context;
    return 0;
}