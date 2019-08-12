//
// Created by 何士奇 on 2019-08-06.
//
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#ifndef VIDEO_EDITOR_FFMPEG_UTILS_H
#define VIDEO_EDITOR_FFMPEG_UTILS_H
/**
 *  打开输入文件  并创建输入 AVFormatContext AVCodecContext
 * @param filename 所要打开的文件
 * @param input_format_context
 * @param input_codec_context
 * @return
 */
int open_input_file(const char *filename,AVFormatContext **input_format_context);

int find_stream_index(AVFormatContext *input_format_context,int *video_index,int *audio_index);

int init_input_codec(AVFormatContext **input_format_context,AVCodecContext **codec_context,int *stream_index,enum AVMediaType type);

int open_output_file(const char *filename,AVFormatContext **output_format_context);
int open_output_file_with_oformat(const char *filename,AVFormatContext **output_format_context, char *oformat);

int init_output_codec(AVCodecContext *input_codec_context,AVFormatContext **output_format_context,AVCodecContext **codec_context,enum AVMediaType type);

int copy_output_codec(AVCodecContext *input_codec_context,AVFormatContext **input_format_context,AVFormatContext **output_format_context,AVCodecContext **codec_context,int stream_index);

void init_packet(AVPacket *packet);

int init_input_frame(AVFrame **frame);

int write_output_file_header(AVFormatContext *output_format_context);
int write_output_file_trailer(AVFormatContext *output_format_context);
#endif //VIDEO_EDITOR_FFMPEG_UTILS_H
