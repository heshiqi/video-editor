//
// Created by heshiqi on 2019-08-04.
//
#include <android/log.h>

#ifndef VIDEO_EDITOR_MASTER_ANDROID_LOG_H
#define VIDEO_EDITOR_MASTER_ANDROID_LOG_H

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "hh", __VA_ARGS__)

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "hh", __VA_ARGS__)

#endif //VIDEO_EDITOR_MASTER_ANDROID_LOG_H
