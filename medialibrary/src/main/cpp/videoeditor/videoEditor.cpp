//
// Created by 何士奇 on 2019-08-01.
//

#include "videoEditor.h"
#include <string>
#include <string.h>
#include <sstream>
#include<stdio.h>
#include <android/log.h>


extern "C" {
#include "common/ffmpeg_utils.h"
#include "video2image.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavcodec/avfft.h>
}

#include "../android_log.h"

//const char *LOG_TAG = "hh";
//#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
//
//#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

void extract_video_audio(char *in_filename, char *out_filename_audio, char *out_filename_video) {

    AVFormatContext *input_format_context = NULL, *ofmt_ctx_video = NULL, *ofmt_ctx_audio = NULL;
    AVCodecContext *input_video_codec_context = NULL, *input_audio_codec_context = NULL, *output_video_codec_context = NULL, *output_audio_codec_context = NULL;

    int videoindex = -1;
    int audioindex = -1;

    av_register_all();
    if (open_input_file(in_filename, &input_format_context)) {
        goto cleanup;
    }
    if (init_input_codec(&input_format_context, &input_video_codec_context, &videoindex,
                         AVMEDIA_TYPE_VIDEO)) {
        goto cleanup;
    }
    if (init_input_codec(&input_format_context, &input_audio_codec_context, &audioindex,
                         AVMEDIA_TYPE_AUDIO)) {
        goto cleanup;
    }
    if (open_output_file(out_filename_video, &ofmt_ctx_video)) {
        goto cleanup;
    }
    if (copy_output_codec(input_video_codec_context, &input_format_context, &ofmt_ctx_video,
                          &output_video_codec_context, videoindex)) {
        goto cleanup;
    }
    if (open_output_file(out_filename_audio, &ofmt_ctx_audio)) {
        goto cleanup;
    }
    if (copy_output_codec(input_audio_codec_context, &input_format_context, &ofmt_ctx_audio,
                          &output_audio_codec_context, audioindex)) {
        goto cleanup;
    }
    if (write_output_file_header(ofmt_ctx_video)) {
        goto cleanup;
    }
    if (write_output_file_header(ofmt_ctx_audio)) {
        goto cleanup;
    }

    AVPacket pkt;
    int frame_index;
    // 分离某些封装格式（例如MP4/FLV/MKV等）中的H.264的时候，需要首先写入SPS和PPS，否则会导致分离出来的数据
    // 没有SPS、PPS而无法播放。使用ffmpeg中名称为“h264_mp4toannexb”的bitstream filter处理
//    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
    while (1) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        if (av_read_frame(input_format_context, &pkt) < 0)
            break;
        in_stream = input_format_context->streams[pkt.stream_index];
        // stream_index标识该AVPacket所属的视频/音频流
        if (pkt.stream_index == videoindex) {
            // 前面已经通过avcodec_copy_context()函数把输入视频/音频的参数拷贝至输出视频/音频的AVCodecContext结构体
            // 所以使用的就是ofmt_ctx_video的第一个流streams[0]
            out_stream = ofmt_ctx_video->streams[0];
            ofmt_ctx = ofmt_ctx_video;
            LOGD("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
//            av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size,
//                                       pkt.data, pkt.size, 0);
        } else if (pkt.stream_index == audioindex) {
            out_stream = ofmt_ctx_audio->streams[0];
            ofmt_ctx = ofmt_ctx_audio;
            LOGD("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
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
            LOGE("Error muxing packet\n");
            break;
        }
        LOGD("Write %8d frames to output file\n", frame_index);
        av_packet_unref(&pkt);
        frame_index++;
    }

    if (write_output_file_trailer(ofmt_ctx_video))
        goto cleanup;
    if (write_output_file_trailer(ofmt_ctx_audio))
        goto cleanup;

    cleanup:
    if (output_video_codec_context) {
        avcodec_free_context(&output_video_codec_context);
    }
    if (output_audio_codec_context) {
        avcodec_free_context(&output_audio_codec_context);
    }
    if (ofmt_ctx_video) {
        avio_closep(&ofmt_ctx_video->pb);
        avformat_free_context(ofmt_ctx_video);
    }
    if (ofmt_ctx_audio) {
        avio_closep(&ofmt_ctx_audio->pb);
        avformat_free_context(ofmt_ctx_audio);
    }
    if (input_video_codec_context)
        avcodec_free_context(&input_video_codec_context);
    if (input_audio_codec_context)
        avcodec_free_context(&input_audio_codec_context);
    if (input_format_context)
        avformat_close_input(&input_format_context);
    LOGD("执行完成");
}

JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_extractAudio(JNIEnv *env, jobject instance,
                                                               jstring path_, jstring videoPath_,
                                                               jstring audioPath_) {

    char *in_filename = (char *) env->GetStringUTFChars(path_, 0);
    char *out_filename_audio = (char *) env->GetStringUTFChars(audioPath_, 0);
    char *out_filename_video = (char *) env->GetStringUTFChars(videoPath_, 0);
    LOGE("source file %s ", in_filename);
    extract_video_audio(in_filename, out_filename_audio, out_filename_video);
}

JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_mergeVideoAudio(JNIEnv *env, jobject instance,
                                                                  jstring videoPath_,
                                                                  jstring audioPath_,
                                                                  jstring outputPath_) {
    AVOutputFormat *ofmt = NULL;

    AVFormatContext *ifmt_ctx_v = NULL;//视频输入 AVFormatContext
    AVFormatContext *ifmt_ctx_a = NULL;//音频输入 AVFormatContext
    AVFormatContext *ofmt_ctx = NULL;//音视频输入 AVFormatContext
    int ret, i;

    int videoindex_v = -1, videoindex_out = -1;
    int audioindex_a = -1, audioindex_out = -1;
    int frame_index = 0;
    int64_t cur_pts_v = 0, cur_pts_a = 0;
    const char *musicPath = env->GetStringUTFChars(audioPath_, 0);
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    const char *outPath = env->GetStringUTFChars(outputPath_, 0);

    av_register_all();

    //--------------------------------音视频输入初始化处理 开始---------------------------------------------

    if ((ret = avformat_open_input(&ifmt_ctx_v, videoPath, 0, 0)) < 0) {//打开输入的视频文件
        LOGE("Could not open input video file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {//获取视频文件信息
        LOGE("Failed to retrieve input video stream information");
        goto end;
    }
    if ((ret = avformat_open_input(&ifmt_ctx_a, musicPath, 0, 0)) < 0) {//打开输入的音频文件
        LOGD("Could not open input audio file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {//获取音频文件信息
        LOGE("Failed to retrieve input audio stream information");
        goto end;
    }

    //    LOGD("===========输入音视频 信息==========\n");
    //    av_dump_format(ifmt_ctx_v, 0, videoPath, 0);
    //    av_dump_format(ifmt_ctx_a, 0, musicPath, 0);
    //    LOGD("======================================\n");

    //--------------------------------音视频输入初始化处理 结束---------------------------------------------

    //--------------------------------输出影视频初始化处理 开始---------------------------------------------
    //初始化输出码流的AVFormatContext
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, outPath);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    //--------------------------------输出影视频初始化处理 结束---------------------------------------------

    //--------------------------------相关值获取-----------------------------------------------
    //从输入video的AVStream中获取一个video输出的out_stream
    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        if (ifmt_ctx_v->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {//输入流类型为视频
            AVStream *in_stream = ifmt_ctx_v->streams[i];
            AVCodec *dec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            if (!dec) {
                LOGE("Could not find decoder\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, dec);
            videoindex_v = i;//输入视频流位置索引赋值
            if (!out_stream) {
                LOGE("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }

            //      -----------------------赋值视频输入流信息到输出流中 开始--------------------------------
            videoindex_out = out_stream->index;
            AVCodecContext *avCodecContext = avcodec_alloc_context3(dec);
            if ((ret = avcodec_parameters_to_context(avCodecContext, in_stream->codecpar)) < 0) {
                avcodec_free_context(&avCodecContext);
                avCodecContext = NULL;
                LOGE("can not fill decodecctx");
                goto end;
            }
            avCodecContext->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
                avCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
            ret = avcodec_parameters_from_context(out_stream->codecpar, avCodecContext);
            if (ret < 0) {
                printf("Failed to copy context input to output stream codec context\n");
                goto end;
            }
            //      -----------------------赋值视频输入流信息到输出流中 结束--------------------------------
            break;
        }
    }

    //从输入audio的AVStream中获取一个audio输出的out_stream
    for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        if (ifmt_ctx_a->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {//输入流类型为音频
            AVStream *in_stream = ifmt_ctx_a->streams[i];
            AVCodec *dec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            if (!dec) {
                LOGE("Could not find decoder\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, dec);
            audioindex_a = i;//输入音频流位置索引赋值
            if (!out_stream) {
                LOGE("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            //      -----------------------赋值音频输入流信息到输出流中 开始--------------------------------
            audioindex_out = out_stream->index;
            AVCodecContext *avCodecContext = avcodec_alloc_context3(dec);
            if ((ret = avcodec_parameters_to_context(avCodecContext, in_stream->codecpar)) < 0) {
                avcodec_free_context(&avCodecContext);
                avCodecContext = NULL;
                LOGE("can not fill decodecctx");
                goto end;
            }
            avCodecContext->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
                avCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
            ret = avcodec_parameters_from_context(out_stream->codecpar, avCodecContext);
            if (ret < 0) {
                printf("Failed to copy context input to output stream codec context\n");
                goto end;
            }
            //      -----------------------赋值音频输入流信息到输出流中 结束--------------------------------
            break;
        }
    }

    //    -------------------------------合成文件-------------------------------------------

    // 打开输出文件
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, outPath, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Could not open output file %s ", outPath);
            goto end;
        }
    }

    // 写头文件
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGD("Error occurred when opening output file\n");
        goto end;
    }

    while (1) {
        AVFormatContext *ifmt_ctx;
        int stream_index = 0;
        AVStream *in_stream, *out_stream;
        AVPacket *pkt = av_packet_alloc();

        //Get an AVPacket .   av_compare_ts是比较时间戳用的。通过该函数可以决定该写入视频还是音频。
        //video 在 audio之前
        if (av_compare_ts(cur_pts_v,
                          ifmt_ctx_v->streams[videoindex_v]->time_base,
                          cur_pts_a,
                          ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0) {
            ifmt_ctx = ifmt_ctx_v;
            stream_index = videoindex_out;
            if (av_read_frame(ifmt_ctx, pkt) >= 0) {
                do {
                    if (pkt->stream_index == videoindex_v) {
                        in_stream = ifmt_ctx->streams[pkt->stream_index];
                        out_stream = ofmt_ctx->streams[stream_index];
                        //FIX：No PTS (Example: Raw H.264) H.264裸流没有PTS，因此必须手动写入PTS
                        //Simple Write PTS
                        if (pkt->pts == AV_NOPTS_VALUE) {
                            //Write PTS
                            AVRational time_base1 = in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration =
                                    (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt->pts = (double) (frame_index * calc_duration) /
                                       (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt->dts = pkt->pts;
                            pkt->duration = (double) calc_duration /
                                            (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }

                        cur_pts_v = pkt->pts;
                        break;
                    }
                } while (av_read_frame(ifmt_ctx, pkt) >= 0);
            } else {
                av_packet_free(&pkt);
                av_free(pkt);
                break;
            }
        } else {
            //如果video在audio之后
            ifmt_ctx = ifmt_ctx_a;
            stream_index = audioindex_out;
            if (av_read_frame(ifmt_ctx, pkt) >= 0) {
                do {
                    if (pkt->stream_index == audioindex_a) {
                        in_stream = ifmt_ctx->streams[pkt->stream_index];
                        out_stream = ofmt_ctx->streams[stream_index];
                        //FIX：No PTS
                        //Simple Write PTS
                        LOGE("pkt->pts--------------");
                        if (pkt->pts == AV_NOPTS_VALUE) {
                            LOGE("pkt->pts==AV_NOPTS_VALUE\n");
                            //Write PTS
                            AVRational time_base1 = in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration =
                                    (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt->pts = (double) (frame_index * calc_duration) /
                                       (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt->dts = pkt->pts;
                            pkt->duration = (double) calc_duration /
                                            (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_a = pkt->pts;
                        break;
                    }
                } while (av_read_frame(ifmt_ctx, pkt) >= 0);
            } else {
                av_packet_free(&pkt);
                av_free(pkt);
                break;
            }
        }

        //Convert PTS/DTS
        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base,
                                    (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base,
                                    (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;
        pkt->stream_index = stream_index;

        LOGD("Write 1 Packet. size:%5d\tpts:%lld\n", pkt->size, pkt->pts);
        //Write AVPacket 音频或视频裸流
        int result = av_interleaved_write_frame(ofmt_ctx, pkt);
        LOGD("result:%5d", result);
        if (result < 0) {
            LOGD("Error muxing packet\n");
            av_packet_free(&pkt);
            av_free(pkt);
            break;
        }
        av_packet_free(&pkt);
        av_free(pkt);
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);
    end:
    avformat_close_input(&ifmt_ctx_v);
    avformat_close_input(&ifmt_ctx_a);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    env->ReleaseStringUTFChars(audioPath_, musicPath);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
    env->ReleaseStringUTFChars(outputPath_, outPath);
    LOGD("执行完成");
}

JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_video2Images(JNIEnv *env, jobject instance,
                                                               jstring inFilePath_,
                                                               jstring outFilePath_) {
    char *in_filename = (char *) env->GetStringUTFChars(inFilePath_, 0);
    char *out_filename = (char *) env->GetStringUTFChars(outFilePath_, 0);
    video_extract_images(in_filename, out_filename);
}



