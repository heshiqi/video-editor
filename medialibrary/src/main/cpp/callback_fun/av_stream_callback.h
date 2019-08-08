//
// Created by 何士奇 on 2019-08-07.
//
#include <libavformat/avformat.h>

#ifndef VIDEO_EDITOR_AV_STREAM_CALLBACK_H
#define VIDEO_EDITOR_AV_STREAM_CALLBACK_H

void on_av_codec_context(AVFormatContext *formatContext, AVStream *stream, AVFrame *pFrame);

int Stream_Handle(int y, void (*Callback)(AVStream *stream));

#endif //VIDEO_EDITOR_AV_STREAM_CALLBACK_H
