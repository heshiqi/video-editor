//
// Created by heshiqi on 2019-08-04.
//
#include "video_cut.h"
#include "android_log.h"

extern "C" {
#include <libavutil/time.h>
//#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#include <libavutil/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h> // time_t, tm, time, localtime, strftime
#include <sys/time.h> // time_t, tm, time, localtime, strftime
}

static int
encode_and_save_pkt(AVCodecContext *enc_ctx, AVFormatContext *ofmt_ctx, AVStream *in_stream,
                    AVStream *out_stream) {
    AVPacket enc_pkt;
    av_init_packet(&enc_pkt);
    enc_pkt.data = NULL;
    enc_pkt.size = 0;

    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, &enc_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            break;
        } else if (ret < 0) {
            LOGE("[avcodec_receive_packet]Error during encoding, ret:%d\n", ret);
            break;
        }
        LOGD("encode type:%d pts:%d dts:%d\n", enc_ctx->codec_type, enc_pkt.pts, enc_pkt.dts);


        //Convert PTS/DTS
        enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        enc_pkt.duration = av_rescale_q(enc_pkt.duration, in_stream->time_base,
                                        out_stream->time_base);
        enc_pkt.pos = -1;
        enc_pkt.stream_index = out_stream->index;
//        /* rescale output packet timestamp values from codec to stream timebase */
//        av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base, out_stream->time_base);
//        enc_pkt.stream_index = out_stream->index;
        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
        if (ret < 0) {
            LOGE("write frame error, ret:%d\n", ret);
            break;
        }

        av_packet_unref(&enc_pkt);
    }
    return ret;
}

static int
decode_and_send_frame(AVCodecContext *dec_ctx, AVCodecContext *enc_ctx, int64_t start, int64_t end,
                      int time_base_dn) {
    AVFrame *frame = av_frame_alloc();
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            break;
        } else if (ret < 0) {
            LOGE("Error while receiving a frame from the decoder");
            break;
        }
        int pts = frame->pts;
        LOGD("decode type:%d pts:%d\n", dec_ctx->codec_type, pts);
        if ((pts < start * time_base_dn) || (pts > end * time_base_dn)) {  // 数据裁剪
            continue;
        }
//        frame->pict_type = AV_PICTURE_TYPE_NONE;
//        // 修改pts
        frame->pts = pts - start * time_base_dn;

        ret = avcodec_send_frame(enc_ctx, frame);
        if (ret < 0) {
            LOGE("Error sending a frame for encoding\n");
            break;
        }
    }
    av_frame_free(&frame);
    return ret;
}

static int open_decodec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                                enum AVMediaType type) {
    int ret;
    int stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
    if (ret < 0) {
        LOGE("Could not find %s stream in input file \n", av_get_media_type_string(type));
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            LOGE("Failed to allocate the %s codec context\n", av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            LOGE("Failed to copy %s codec parameters to decoder context\n",
                 av_get_media_type_string(type));
            return ret;
        }

        if ((*dec_ctx)->codec_type == AVMEDIA_TYPE_VIDEO)
            (*dec_ctx)->framerate = av_guess_frame_rate(fmt_ctx, st, NULL);

        /* Init the decoders, with or without reference counting */
        av_dict_set_int(&opts, "refcounted_frames", 1, 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            LOGE("Failed to open %s codec\n", av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

static int set_encode_option(AVCodecContext *dec_ctx, AVDictionary **opt) {
    const char *profile = avcodec_profile_name(dec_ctx->codec_id, dec_ctx->profile);
    if (profile) {
        if (!strcasecmp(profile, "high")) {
            av_dict_set(opt, "profile", "high", 0);
        }
    } else {
        av_dict_set(opt, "profile", "main", 0);
    }

    av_dict_set(opt, "threads", "16", 0);

    av_dict_set(opt, "preset", "slow", 0);
    av_dict_set(opt, "level", "4.0", 0);

    return 0;
}

static int
open_encodec_context(int stream_index, AVCodecContext *dec_ctx, AVCodecContext **oenc_ctx,
                     AVFormatContext *fmt_ctx,
                     enum AVMediaType type) {
    int ret;
    AVStream *st;
    AVCodec *encoder = NULL;
    AVDictionary *opts = NULL;
    AVCodecContext *enc_ctx;

    st = fmt_ctx->streams[stream_index];
    /* find encoder for the stream */
    encoder = avcodec_find_encoder(st->codecpar->codec_id);
    if (!encoder) {
        LOGE("Failed to find %s codec\n", av_get_media_type_string(type));
        return AVERROR(EINVAL);
    }

    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        LOGE("Failed to allocate the encoder context\n");
        return AVERROR(ENOMEM);
    }

//    AVCodecContext *dec_ctx = st->codec;
    if (type == AVMEDIA_TYPE_VIDEO) {
        enc_ctx->height = st->codecpar->height;
        enc_ctx->width = st->codecpar->width;
        enc_ctx->sample_aspect_ratio = st->codecpar->sample_aspect_ratio;

        enc_ctx->bit_rate = st->codecpar->bit_rate;
        enc_ctx->rc_max_rate = st->codecpar->bit_rate;
        enc_ctx->rc_buffer_size = st->codecpar->bit_rate;
        enc_ctx->bit_rate_tolerance = 0;
        // use yuv420P
        enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        // set frame rate
//        enc_ctx->time_base.num = 1;
//        enc_ctx->time_base.den = TIME_DEN;
        enc_ctx->time_base = st->time_base;
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        enc_ctx->has_b_frames = false;
        enc_ctx->max_b_frames = 0;
        enc_ctx->gop_size = 120;

        set_encode_option(dec_ctx, &opts);
    } else if (type == AVMEDIA_TYPE_AUDIO) {
        enc_ctx->sample_rate = st->codecpar->sample_rate;
        enc_ctx->channel_layout = st->codecpar->channel_layout;
        enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
        /* take first format from list of supported formats */
        enc_ctx->sample_fmt = encoder->sample_fmts[0];
        enc_ctx->time_base = st->time_base;
        enc_ctx->bit_rate = st->codecpar->bit_rate;
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    } else {
        ret = avcodec_copy_context(enc_ctx, st->codec);
        if (ret < 0) {
            LOGE("Failed to copy context from input to output stream codec context\n");
            return ret;
        }
//        if (avcodec_parameters_to_context(enc_ctx, st->codecpar) < 0) {
//            LOGE("Failed to copy context from input to output stream codec context.");
//            return ret;
//        }
    }

    if ((ret = avcodec_open2(enc_ctx, encoder, &opts)) < 0) {
        LOGE("Failed to open %s codec\n", av_get_media_type_string(type));
        return ret;
    }

    *oenc_ctx = enc_ctx;
    return 0;
}

int test_cut(char *input_file, char *output_file, int start, int end) {
    int ret = 0;

    // 输入流
    AVFormatContext *ifmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL;
    AVCodecContext *audio_dec_ctx = NULL;

    int video_stream_idx = 0;
    int audio_stream_idx = 1;

    // 输出流
    AVFormatContext *ofmt_ctx = NULL;
    AVCodecContext *audio_enc_ctx = NULL;
    AVCodecContext *video_enc_ctx = NULL;

    int videotime_base_dn;
    int audiotime_base_dn;

    if ((ret = avformat_open_input(&ifmt_ctx, input_file, 0, 0)) < 0) {
        LOGE("Could not open input file '%s' ret:%d\n", input_file, ret);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        goto end;
    }

    if (open_decodec_context(&video_stream_idx, &video_dec_ctx, ifmt_ctx, AVMEDIA_TYPE_VIDEO) < 0) {
        LOGE("fail to open vedio decode context, ret:%d\n", ret);
        goto end;
    }
    if (open_decodec_context(&audio_stream_idx, &audio_dec_ctx, ifmt_ctx, AVMEDIA_TYPE_AUDIO) < 0) {
        LOGE("fail to open audio decode context, ret:%d\n", ret);
        goto end;
    }

    // 设置输出
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_file);
    if (!ofmt_ctx) {
        LOGE("can not open ouout context");
        goto end;
    }

    // video stream
    AVStream *out_stream;
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream) {
        LOGE("Failed allocating output stream\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    if ((ret = open_encodec_context(video_stream_idx, video_dec_ctx, &video_enc_ctx, ifmt_ctx,
                                    AVMEDIA_TYPE_VIDEO)) < 0) {
        LOGE("video enc ctx init err\n");
        goto end;
    }
    ret = avcodec_parameters_from_context(out_stream->codecpar, video_enc_ctx);
    if (ret < 0) {
        LOGE("Failed to copy codec parameters\n");
        goto end;
    }

    video_enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    out_stream->time_base = video_enc_ctx->time_base;
    out_stream->codecpar->codec_tag = 0;
    videotime_base_dn = out_stream->time_base.den;
    // audio stream
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream) {
        LOGE("Failed allocating output stream\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    if ((ret = open_encodec_context(audio_stream_idx, audio_dec_ctx, &audio_enc_ctx, ifmt_ctx,
                                    AVMEDIA_TYPE_AUDIO)) < 0) {
        LOGE("audio enc ctx init err\n");
        goto end;
    }
    ret = avcodec_parameters_from_context(out_stream->codecpar, audio_enc_ctx);
    if (ret < 0) {
        LOGE("Failed to copy codec parameters\n");
        goto end;
    }
    audio_enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    out_stream->time_base = audio_enc_ctx->time_base;
    out_stream->codecpar->codec_tag = 0;
    audiotime_base_dn = out_stream->time_base.den;

    // 打开文件
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, output_file, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Could not open output file '%s'\n", output_file);
            goto end;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("Error occurred when opening output file\n");
        goto end;
    }

    AVPacket flv_pkt;
    int stream_index;
    while (1) {
        AVPacket *pkt = &flv_pkt;
        ret = av_read_frame(ifmt_ctx, pkt);
        if (ret < 0) {
            break;
        }

        if (pkt->stream_index == video_stream_idx && (pkt->flags & AV_PKT_FLAG_KEY)) {
            LOGD("pkt.dts = %ld pkt.pts = %ld pkt.stream_index = %d is key frame\n", pkt->dts,
                 pkt->pts, pkt->stream_index);
        }
        stream_index = pkt->stream_index;

        if (pkt->stream_index == video_stream_idx) {
            ret = avcodec_send_packet(video_dec_ctx, pkt);
            LOGD("read type:%d pts:%d dts:%d\n", video_dec_ctx->codec_type, pkt->pts, pkt->dts);

            ret = decode_and_send_frame(video_dec_ctx, video_enc_ctx, start, end,
                                        videotime_base_dn);
            ret = encode_and_save_pkt(video_enc_ctx, ofmt_ctx, ifmt_ctx->streams[video_stream_idx],
                                      ofmt_ctx->streams[0]);
            if (ret < 0) {
                LOGE("re encode video error, ret:%d\n", ret);
            }
        } else if (pkt->stream_index == audio_stream_idx) {
            ret = avcodec_send_packet(audio_dec_ctx, pkt);
            LOGD("read type:%d pts:%d dts:%d\n", audio_dec_ctx->codec_type, pkt->pts, pkt->dts);
            ret = decode_and_send_frame(audio_dec_ctx, audio_enc_ctx, start, end,
                                        audiotime_base_dn);
            ret = encode_and_save_pkt(audio_enc_ctx, ofmt_ctx, ifmt_ctx->streams[audio_stream_idx],
                                      ofmt_ctx->streams[1]);
            if (ret < 0) {
                LOGE("re encode audio error, ret:%d\n", ret);
            }
        }
        av_packet_unref(pkt);
    }
    // fflush
    // fflush encode
    avcodec_send_packet(video_dec_ctx, NULL);
    decode_and_send_frame(video_dec_ctx, video_enc_ctx, start, end, videotime_base_dn);
    avcodec_send_packet(audio_dec_ctx, NULL);
    decode_and_send_frame(audio_dec_ctx, audio_enc_ctx, start, end, audiotime_base_dn);
    // fflush decode
    avcodec_send_frame(video_enc_ctx, NULL);
    encode_and_save_pkt(video_enc_ctx, ofmt_ctx, ifmt_ctx->streams[video_stream_idx],
                        ofmt_ctx->streams[0]);
    avcodec_send_frame(audio_enc_ctx, NULL);
    encode_and_save_pkt(audio_enc_ctx, ofmt_ctx, ifmt_ctx->streams[audio_stream_idx],
                        ofmt_ctx->streams[1]);
    LOGD("stream end\n");
    av_write_trailer(ofmt_ctx);

    end:
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    avcodec_free_context(&video_dec_ctx);
    avcodec_free_context(&audio_dec_ctx);

    return ret;
}

JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_videoCut(JNIEnv *env, jobject instance,
                                                           jstring inFilePath_,
                                                           jstring outFilePath_, jint start,
                                                           jint end) {
    av_register_all();
    char *input_file = (char *) env->GetStringUTFChars(inFilePath_, 0);
    char *output_file = (char *) env->GetStringUTFChars(outFilePath_, 0);
    test_cut(input_file, output_file, start, end);

    //  AVRational default_timebase;
    //    default_timebase.num = 1;
    //    default_timebase.den = AV_TIME_BASE;
    //
    //    // suppose you have access to the "inVideoStream" of course
//        int64_t starttime_int64 = av_rescale_q((int64_t)( starttime * AV_TIME_BASE ), default_timebase, inVideoStream->time_base);
    //    int64_t endtime_int64 = av_rescale_q((int64_t)( endtime * AV_TIME_BASE ), default_timebase, inVideoStream->time_base);
}
