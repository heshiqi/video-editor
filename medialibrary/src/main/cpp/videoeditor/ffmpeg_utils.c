//
// Created by 何士奇 on 2019-08-06.
//
#include "android_log.h"
#include "ffmpeg_utils.h"

int open_input_file(const char *filename, AVFormatContext **input_format_context) {

    int error;

    /* 打开要读取的输入文件 */
    if ((error = avformat_open_input(input_format_context, filename, NULL, NULL)) < 0) {
        LOGE("不能打开文件 '%s' (error '%s')\n", filename, av_err2str(error));
        *input_format_context = NULL;
        return error;
    }

    /* 读取输入文件的流信息 */
    if ((error = avformat_find_stream_info(*input_format_context, NULL)) < 0) {
        LOGE("不能获取流信息 (错误log '%s')\n", av_err2str(error));
        avformat_close_input(input_format_context);
        return error;
    }
    return 0;
}

int
init_input_codec(AVFormatContext **input_format_context, AVCodecContext **codec_context,
                 int *stream_index, enum AVMediaType type) {
    int error;

    AVCodecContext *input_av_codecContext;
    AVCodec *input_av_codec;
    int index;
    // 一般情况下nb_streams只有两个流，就是streams[0],streams[1]，分别是音频和视频流，不过顺序不定
    for (int i = 0; i < (*input_format_context)->nb_streams; ++i) {
        if ((*input_format_context)->streams[i]->codecpar->codec_type == type) {
            index = i;
            break;
        }
    }

    /* 获取解码器 */
    if (!(input_av_codec = avcodec_find_decoder(
            (*input_format_context)->streams[index]->codecpar->codec_id))) {
        LOGE("不能获取视频解码器\n");
        avformat_close_input(input_format_context);
        return AVERROR_EXIT;
    }
    /* 创建解码器上下文 */
    input_av_codecContext = avcodec_alloc_context3(input_av_codec);
    if (!input_av_codecContext) {
        LOGE("不能创建解码器上下文\n");
        avformat_close_input(input_format_context);
        return AVERROR(ENOMEM);
    }

    /* 初始化流参数 */
    error = avcodec_parameters_to_context(input_av_codecContext,
                                          (*input_format_context)->streams[index]->codecpar);
    if (error < 0) {
        LOGE("初始化参数错误\n");
        avformat_close_input(input_format_context);
        avcodec_free_context(&input_av_codecContext);
        return error;
    }
    /*
     * 打开流的解码器以便稍后使用
     * */
    if ((error = avcodec_open2(input_av_codecContext, input_av_codec, NULL)) < 0) {
        LOGE("不能打开解码器 (错误 '%s')\n",
             av_err2str(error));
        avcodec_free_context(&input_av_codecContext);
        avformat_close_input(input_format_context);
        return error;
    }

    /* 赋值解码器便于后面使用*/
    *codec_context = input_av_codecContext;

    *stream_index = index;
    return 0;
}

int open_output_file(const char *filename, AVFormatContext **output_format_context) {
    int error;
    AVIOContext *output_io_context = NULL;
    /* 打开输出文件 */
    if ((error = avio_open(&output_io_context, filename,
                           AVIO_FLAG_WRITE)) < 0) {
        LOGE("不能打开输出文件 '%s' (error '%s')\n", filename, av_err2str(error));
        return error;
    }

    /* 创建输出容器格式上下文 */
    if (!(*output_format_context = avformat_alloc_context())) {
        LOGE("不能创建输出容器格式上下文\n");
        return AVERROR(ENOMEM);
    }

    /* 输出文件指针与容器格式上下文关联 */
    (*output_format_context)->pb = output_io_context;

    /* 根据文件扩展名猜测所需的容器格式 */
    if (!((*output_format_context)->oformat = av_guess_format(NULL, filename, NULL))) {
        LOGE("不能获取到输出文件格式\n");
        goto cleanup;
    }
    return 0;
    cleanup:
    avformat_free_context(*output_format_context);
    return error < 0 ? error : AVERROR_EXIT;
}

int open_output_file_with_oformat(const char *filename,AVFormatContext **output_format_context, char *oformat){
    int error;
    AVIOContext *output_io_context = NULL;
    /* 打开输出文件 */
    if ((error = avio_open(&output_io_context, filename,
                           AVIO_FLAG_WRITE)) < 0) {
        LOGE("不能打开输出文件 '%s' (error '%s')\n", filename, av_err2str(error));
        return error;
    }

    /* 创建输出容器格式上下文 */
    if (!(*output_format_context = avformat_alloc_context())) {
        LOGE("不能创建输出容器格式上下文\n");
        return AVERROR(ENOMEM);
    }

    /* 输出文件指针与容器格式上下文关联 */
    (*output_format_context)->pb = output_io_context;

    /* 根据文件扩展名猜测所需的容器格式 */
    if (!((*output_format_context)->oformat = av_guess_format(oformat, NULL, NULL))) {
        LOGE("不能获取到输出文件格式\n");
        goto cleanup;
    }
    return 0;
    cleanup:
    avformat_free_context(*output_format_context);
    return error < 0 ? error : AVERROR_EXIT;
}

int init_output_codec(AVCodecContext *input_codec_context, AVFormatContext **output_format_context,
                      AVCodecContext **codec_context, enum AVMediaType type) {
    int error;

    AVCodecContext *avctx = NULL;
    AVCodec *output_codec = NULL;
    AVStream *stream = NULL;
    /* 按名称查找使用的编码器 */
    if (!(output_codec = avcodec_find_encoder(input_codec_context->codec_id))) {
        LOGE("不能找到指定的编码器\n");
        goto cleanup;
    }
    /* 在输出文件容器中创建新的流. */
    if (!(stream = avformat_new_stream(*output_format_context, NULL))) {
        LOGE("不能创建新的流\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    avctx = avcodec_alloc_context3(output_codec);
    if (!avctx) {
        LOGE("不能创建编码器上下文\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    if (type == AVMEDIA_TYPE_VIDEO) {
        avctx->height = input_codec_context->height;
        avctx->width = input_codec_context->width;
        avctx->sample_aspect_ratio = input_codec_context->sample_aspect_ratio;

        avctx->bit_rate = input_codec_context->bit_rate;
        avctx->rc_max_rate = input_codec_context->bit_rate;
        avctx->rc_buffer_size = input_codec_context->bit_rate;
        avctx->bit_rate_tolerance = 0;
        // use yuv420P
        avctx->pix_fmt = AV_PIX_FMT_YUV420P;
        // set frame rate
        avctx->time_base = input_codec_context->time_base;
        avctx->has_b_frames = input_codec_context->has_b_frames;
        avctx->max_b_frames = input_codec_context->max_b_frames;
        avctx->gop_size = input_codec_context->gop_size;
        stream->time_base.den = input_codec_context->time_base.den;
        stream->time_base.num = input_codec_context->time_base.num;
    } else if (type == AVMEDIA_TYPE_AUDIO) {
        avctx->channels = input_codec_context->channels;
        avctx->channel_layout = input_codec_context->channel_layout;
        avctx->sample_rate = input_codec_context->sample_rate;
        avctx->sample_fmt = output_codec->sample_fmts[0];
        avctx->bit_rate = input_codec_context->bit_rate;
        avctx->time_base = input_codec_context->time_base;

        /* Set the sample rate for the container. */
        stream->time_base.den = input_codec_context->time_base.den;
        stream->time_base.num = input_codec_context->time_base.num;
    } else {
//        if (avcodec_parameters_to_context(avctx, st->codecpar) < 0) {
//            LOGE("Failed to copy context from input to output stream codec context.");
//            goto cleanup;
//        }
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    /* Some container formats (like MP4) require global headers to be present.
      * Mark the encoder so that it behaves accordingly. */
    if ((*output_format_context)->oformat->flags & AVFMT_GLOBALHEADER)
        avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* 打开输出编码器以便后边使用 */
    if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
        LOGE("不能打开输出编码器 (error '%s')\n", av_err2str(error));
        goto cleanup;
    }

    error = avcodec_parameters_from_context(stream->codecpar, avctx);
    if (error < 0) {
        LOGE("Could not initialize stream parameters\n");
        goto cleanup;
    }

    *codec_context = avctx;
    return 0;

    cleanup:
    avcodec_free_context(&avctx);
    avio_closep(&(*output_format_context)->pb);
    avformat_free_context(*output_format_context);
    *output_format_context = NULL;
    return error < 0 ? error : AVERROR_EXIT;
}

int copy_output_codec(AVCodecContext *input_codec_context,AVFormatContext **input_format_context, AVFormatContext **output_format_context,
                      AVCodecContext **codec_context, int stream_index) {
    int error;

    AVCodecContext *avctx = NULL;
    AVCodec *output_codec = NULL;
    AVStream *stream = NULL;
    /* 按名称查找使用的编码器 */
    if (!(output_codec = avcodec_find_decoder(input_codec_context->codec_id))) {
        LOGE("不能找到指定的编码器\n");
        goto cleanup;
    }
    /* 在输出文件容器中创建新的流. */
    if (!(stream = avformat_new_stream(*output_format_context, NULL))) {
        LOGE("不能创建新的流\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    avctx = avcodec_alloc_context3(output_codec);
    if (!avctx) {
        LOGE("不能创建编码器上下文\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    if (avcodec_parameters_to_context(avctx,
                                      (*input_format_context)->streams[stream_index]->codecpar) <
        0) {
        LOGE("不能复制输入流信息到输出编码器中");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    avctx->codec_tag = 0;
    if ((*output_format_context)->oformat->flags & AVFMT_GLOBALHEADER)
        avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* 打开输出编码器以便后边使用 */
    if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
        LOGE("不能打开输出编码器 (error '%s')\n", av_err2str(error));
        goto cleanup;
    }

    error = avcodec_parameters_from_context(stream->codecpar, avctx);
    if (error < 0) {
        LOGE("Could not initialize stream parameters\n");
        goto cleanup;
    }

    *codec_context = avctx;
    LOGE("codec_context\n");
    return 0;

    cleanup:
    avcodec_free_context(&avctx);
    avio_closep(&(*output_format_context)->pb);
    avformat_free_context(*output_format_context);
    *output_format_context = NULL;
    return error < 0 ? error : AVERROR_EXIT;
}

void init_packet(AVPacket *packet) {
    av_init_packet(packet);
    /* 设置数据包数据和大小，设置为空 */
    packet->data = NULL;
    packet->size = 0;
}

int init_input_frame(AVFrame **frame) {
    if (!(*frame = av_frame_alloc())) {
        LOGE("不能创建帧\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int write_output_file_header(AVFormatContext *output_format_context) {
    int error;
    if ((error = avformat_write_header(output_format_context, NULL)) < 0) {
        LOGE("不能写输出文件头数据 (error '%s')\n", av_err2str(error));
        return error;
    }
    return 0;
}

int write_output_file_trailer(AVFormatContext *output_format_context){
    int error;
    if ((error = av_write_trailer(output_format_context)) < 0) {
        LOGE("不能写输出文件尾部数据 (error '%s')\n",av_err2str(error));
        return error;
    }
    return 0;
}