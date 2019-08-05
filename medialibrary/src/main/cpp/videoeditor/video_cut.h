//
// Created by heshiqi on 2019-08-04.
//
#include <jni.h>
#ifndef VIDEO_EDITOR_MASTER_VIDEO_CUT_H
#define VIDEO_EDITOR_MASTER_VIDEO_CUT_H
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_videoCut(JNIEnv *env, jobject instance,
                                                           jstring inFilePath_,
                                                           jstring outFilePath_, jint start,
                                                           jint end);
#ifdef __cplusplus
}
#endif
#endif
