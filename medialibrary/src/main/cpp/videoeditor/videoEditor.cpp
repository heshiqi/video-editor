//
// Created by 何士奇 on 2019-08-01.
//

#include "videoEditor.h"
#include <string>
#include <string.h>
#include <sstream>
#include<stdio.h>
#include <android/log.h>

const char *LOG_TGA = "hh";

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavcodec/avfft.h>
}

JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_extractAudio(JNIEnv *env, jobject instance,
                                                               jstring path_, jstring videoPath_,
                                                               jstring audioPath_) {

    char *in_filename = (char *) env->GetStringUTFChars(path_, 0);
    char *out_filename_audio = (char *) env->GetStringUTFChars(audioPath_, 0);
    char *out_filename_video = (char *) env->GetStringUTFChars(videoPath_, 0);

//    char audio_name[128] = {0};
//    strcat(audio_name, outputAudioPath);
//    strcat(audio_name, "demuxing_audio.");
//    strcat(audio_name, audioCodec->name);

//    char video_name[128] = {0};
//    strcat(audio_name, outputAudioPath);
//    strcat(audio_name, "demuxing_audio.");
//    strcat(audio_name, audioCodec->name);

    AVFormatContext *ifmt_ctx = NULL;
    AVFormatContext *ofmt_ctx_audio = NULL;
    AVFormatContext *ofmt_ctx_video = NULL;

    AVPacket pkt;

    AVOutputFormat *ofmt_audio = NULL;
    AVOutputFormat *ofmt_video = NULL;

    int videoindex = -1;
    int audioindex = -1;
    int frame_index = 0;

    av_register_all();

    if (avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL)) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Couldn't open input file");
        return;
    }

    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Couldn't find stream information");
        return;
    }

    avformat_alloc_output_context2(&ofmt_ctx_video, NULL, NULL, out_filename_video);

    if (!ofmt_ctx_video) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Couldn't create output context");
        return;
    }
    ofmt_video = ofmt_ctx_video->oformat;

    avformat_alloc_output_context2(&ofmt_ctx_audio, NULL, NULL, out_filename_audio);
    if (!ofmt_ctx_audio) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Couldn't create output context");
        return;
    }
    ofmt_audio = ofmt_ctx_audio->oformat;

    // 一般情况下nb_streams只有两个流，就是streams[0],streams[1]，分别是音频和视频流，不过顺序不定

    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
//        AVFormatContext *ofmt_ctx;
//        AVStream *in_stream = ifmt_ctx->streams[i];
//
//        AVCodec *inCodec = NULL;
//
//        AVStream *out_stream = NULL;
//
//        // 根据音视频类型，根据输入流创建输出流
//        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            videoindex = i;
//            inCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
//            out_stream = avformat_new_stream(ofmt_ctx_video, inCodec);
//            ofmt_ctx = ofmt_ctx_video;
//        } else if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//            audioindex = i;
//            inCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
//            out_stream = avformat_new_stream(ofmt_ctx_video, inCodec);
//            ofmt_ctx = ofmt_ctx_audio;
//        } else {
//
//        }
//
//        if (!out_stream) {
//            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Failed allocating output stream.");
//            return;
//        }
//        if (!inCodec) {
//            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Failed get AVCodec .");
//            return;
//        }
//        // 复制到输出流
//
//        AVCodecContext *codec_ctx = avcodec_alloc_context3(inCodec);
//        if (avcodec_parameters_to_context(codec_ctx, in_stream->codecpar) < 0) {
//            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
//                                "Failed to copy context from input to output stream codec context.");
//            return;
//        }
//        codec_ctx->codec_tag = 0;
//        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
//            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//
//        int ret = avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
//        if (ret < 0) {
//            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
//                                "Failed to copy codec context to out_stream codecpar context .");
//            return;
//        }

        AVFormatContext *ofmt_ctx;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = NULL;
        AVCodec *inCodec = NULL;
        // 根据音视频类型，根据输入流创建输出流
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            inCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            out_stream = avformat_new_stream(ofmt_ctx_video, inCodec);
            ofmt_ctx = ofmt_ctx_video;
        } else if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioindex = i;
            inCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            out_stream = avformat_new_stream(ofmt_ctx_audio, inCodec);
            ofmt_ctx = ofmt_ctx_audio;
        } else
            break;

        if (!out_stream) {
            return;
        }
        // 复制到输出流
        AVCodecParameters *incodecpar = in_stream->codecpar;
        AVCodecParameters *outcodecpar = out_stream->codecpar;

        AVCodecContext *codec_ctx = avcodec_alloc_context3(inCodec);

        if (avcodec_parameters_to_context(codec_ctx, in_stream->codecpar) < 0) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
                                "Failed to copy context from input to output stream codec context.");
            return;
        }
        codec_ctx->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        int ret = avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
        if (ret < 0) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
                                "Failed to copy codec context to out_stream codecpar context .");
            return;
        }
        __android_log_print(ANDROID_LOG_INFO, LOG_TGA,
                            "编码名称======='%s'", inCodec->name);
    }
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA, "\n==============Input Video=============\n");
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA, "\n==============Output Video=============\n");
    av_dump_format(ofmt_ctx_video, 0, out_filename_video, 1);

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA, "\n==============Output Video=============\n");
    av_dump_format(ofmt_ctx_audio, 0, out_filename_audio, 1);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA, "\n======================================\n");

    // 打开输出文件
    if (!(ofmt_video->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx_video->pb, out_filename_video, AVIO_FLAG_WRITE) < 0) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
                                "Couldn't open output file '%s'", out_filename_video);
            return;
        }
    }
    if (!(ofmt_audio->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx_audio->pb, out_filename_audio, AVIO_FLAG_WRITE) < 0) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
                                "Couldn't open output file '%s'", out_filename_video);
            return;
        }
    }

    // 写文件头部
    int r = avformat_write_header(ofmt_ctx_video, NULL);
    if (r < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
                            "Error occurred when opening video output file");
        return;
    }
    r = avformat_write_header(ofmt_ctx_audio, NULL);
    if (r < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TGA,
                            "Error occurred when opening video output file\n");
        return;
    }


    // 分离某些封装格式（例如MP4/FLV/MKV等）中的H.264的时候，需要首先写入SPS和PPS，否则会导致分离出来的数据
    // 没有SPS、PPS而无法播放。使用ffmpeg中名称为“h264_mp4toannexb”的bitstream filter处理
    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
    while (1) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream, *out_stream;

        //Get an AVPacket
        if (av_read_frame(ifmt_ctx, &pkt) < 0)
            break;
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        // stream_index标识该AVPacket所属的视频/音频流
        if (pkt.stream_index == videoindex) {
            // 前面已经通过avcodec_copy_context()函数把输入视频/音频的参数拷贝至输出视频/音频的AVCodecContext结构体
            // 所以使用的就是ofmt_ctx_video的第一个流streams[0]
            out_stream = ofmt_ctx_video->streams[0];
            ofmt_ctx = ofmt_ctx_video;
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA,
                                "Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
            av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size,
                                       pkt.data, pkt.size, 0);
        } else if (pkt.stream_index == audioindex) {
            out_stream = ofmt_ctx_audio->streams[0];
            ofmt_ctx = ofmt_ctx_audio;
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA,
                                "Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
        } else
            continue;

        // DTS（Decoding Time Stamp）解码时间戳
        // PTS（Presentation Time Stamp）显示时间戳
        // 转换PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;

        // 写
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TGA, "Error muxing packet\n");
            break;
        }
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA, "Write %8d frames to output file\n",
                            frame_index);
        av_free_packet(&pkt);
        frame_index++;
    }

    av_bitstream_filter_close(h264bsfc);

    // 写文件尾部
    av_write_trailer(ofmt_ctx_video);
    av_write_trailer(ofmt_ctx_audio);

    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx_video && !(ofmt_video->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_video->pb);

    if (ofmt_ctx_audio && !(ofmt_audio->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_audio->pb);

    avformat_free_context(ofmt_ctx_video);
    avformat_free_context(ofmt_ctx_audio);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TGA, "执行完成");
}

JNIEXPORT jstring JNICALL
Java_com_ah_me_player_videoeditor_VideoEditor_stringFromJNI(JNIEnv *env, jobject instance) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}



