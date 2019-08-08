//
// Created by 何士奇 on 2019-08-07.
//
#include "../callback_fun/av_stream_callback.h"

#ifndef VIDEO_EDITOR_ENTITY_FACTORY_H
#define VIDEO_EDITOR_ENTITY_FACTORY_H

int init_av_codec_context_with_type(AVFormatContext *format_context,AVCodecContext **codec_context,AVCodec **codec,AVFrame *frame,void (*Callback)(AVFormatContext *,AVStream *,AVFrame *));

#endif //VIDEO_EDITOR_ENTITY_FACTORY_H
