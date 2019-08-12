//
// Created by 何士奇 on 2019-08-07.
//
#include "video2image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "../android_log.h"
#include "common/ffmpeg_utils.h"
#include "../factory/entity_factory.h"

void on_av_codec_context(AVFormatContext *formatContext, AVStream *stream, AVFrame *pFrame) {
    AVCodecParameters *parameters = stream->codecpar;
    parameters->codec_id = formatContext->oformat->video_codec;
    parameters->codec_type = AVMEDIA_TYPE_VIDEO;
    parameters->format = AV_PIX_FMT_YUVJ420P;
    parameters->width = pFrame->width;
    parameters->height = pFrame->height;
}


int saveJpg(AVFrame *pFrame, char *out_name) {

    int width = pFrame->width;
    int height = pFrame->height;
    AVCodecContext *pCodeCtx = NULL;
    AVCodec *codec = NULL;
    AVFormatContext *out_format_context;

    if (open_output_file_with_oformat(out_name, &out_format_context, "mjpeg")) {
        goto cleanup;
    }

    if (init_av_codec_context_with_type(out_format_context, &pCodeCtx, &codec, pFrame,
                                        on_av_codec_context)) {
        goto cleanup;
    }

    pCodeCtx->time_base = (AVRational) {1, 25};
    if (avcodec_open2(pCodeCtx, codec, NULL) < 0) {
        LOGE("Could not open codec.");
        goto cleanup;
    }

    if (write_output_file_header(out_format_context)) {
        goto cleanup;
    }

    int y_size = width * height;

    //Encode
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    // 编码数据
    int ret = avcodec_send_frame(pCodeCtx, pFrame);
    if (ret < 0) {
        LOGE("Could not avcodec_send_frame.");
        goto cleanup;
    }

    // 得到编码后数据
    ret = avcodec_receive_packet(pCodeCtx, &pkt);
    if (ret < 0) {
        LOGE("Could not avcodec_receive_packet");
        goto cleanup;
    }


    ret = av_write_frame(out_format_context, &pkt);
    LOGD("write_----------\n");
    if (ret < 0) {
        LOGE("Could not av_write_frame");
        goto cleanup;
    }
    av_packet_unref(&pkt);
    write_output_file_trailer(out_format_context);

    cleanup:
    if (pCodeCtx) {
        avcodec_close(pCodeCtx);
    }
    if (out_format_context) {
        avio_close(out_format_context->pb);
        avformat_free_context(out_format_context);
    }
    return 0;
}

void video_extract_images(char *in_filename, char *out_filename) {
    av_register_all();
    int ret;
    AVFormatContext *in_format_context = NULL;
    AVCodecContext *in_codec_context = NULL;
    int stream_index;
    AVPacket avpkt;
    int frame_count;
    AVFrame *frame;

    if (open_input_file(in_filename, &in_format_context)) {
        goto cleanup;
    }

    if (init_input_codec(&in_format_context, &in_codec_context, &stream_index,
                         AVMEDIA_TYPE_VIDEO)) {
        goto cleanup;
    }

    init_packet(&avpkt);

    //初始化frame，解码后数据
    frame = av_frame_alloc();
    if (!frame) {
        LOGE("Could not allocate video frame\n");
        exit(1);
    }

    frame_count = 0;
    char buf[1024];
    while (av_read_frame(in_format_context, &avpkt) >= 0) {
        if (avpkt.stream_index == stream_index) {

            int re = avcodec_send_packet(in_codec_context, &avpkt);
            if (re < 0) {
                continue;
            }
            //这里必须用while()，因为一次avcodec_receive_frame可能无法接收到所有数据
            while (avcodec_receive_frame(in_codec_context, frame) == 0) {
                // 拼接图片路径、名称
                snprintf(buf, sizeof(buf), "%s/video2img-%d.jpg", out_filename, frame_count);
                saveJpg(frame, buf); //保存为jpg图片
            }
            frame_count++;
        }
        av_packet_unref(&avpkt);
    }

    cleanup:
    if (in_codec_context)
        avcodec_free_context(&in_codec_context);
    if (in_format_context)
        avformat_close_input(&in_format_context);
    LOGD("执行完成");
}




