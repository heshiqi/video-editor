//
// Created by 何士奇 on 2019-08-01.
//
#include <jni.h>

#ifndef FFMPEG_VIDEO_EDITOR_ANDROID_MASTER_COM_AH_ME_PLAYER_PLAYER_VIDEOEDITOR_H
#define FFMPEG_VIDEO_EDITOR_ANDROID_MASTER_COM_AH_ME_PLAYER_PLAYER_VIDEOEDITOR_H

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_ah_me_player_Player
 * Method:    play
 * Signature: (Ljava/lang/String;Landroid/view/Surface;)V
 */
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_videoeditor_VideoEditor_extractAudio(JNIEnv *env, jobject instance,
                                                               jstring path_, jstring videoPath_,
                                                               jstring audioPath_);
#ifdef __cplusplus
}
#endif
#endif
