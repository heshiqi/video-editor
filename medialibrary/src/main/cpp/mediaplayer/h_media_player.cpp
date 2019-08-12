//
// Created by 何士奇 on 2019-08-08.
//
#include <jni.h>
#include "h_java_call.h"
#include "../android_log.h"
#include "h_ffmpeg.h"
#include "h_stataus.h"

extern "C" {
#include <libavcodec/jni.h>
}


_JavaVM *javaVM = NULL;
JavaCall *javaCall = NULL;
HFFmpeg *ffmpeg = NULL;

extern "C" {
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativePrepare(JNIEnv *env, jobject instance, jstring url_, jboolean isOnlyMusic) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (javaCall == NULL) {
        javaCall = new JavaCall(javaVM, env, &instance);
    }
    if (ffmpeg == NULL) {
        ffmpeg = new HFFmpeg(javaCall, url, isOnlyMusic);
        javaCall->onLoad(WL_THREAD_MAIN, true);
        ffmpeg->preparedFFmpeg();
    }

//    env->ReleaseStringUTFChars(url_, url);
}


JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeStart(JNIEnv *env, jobject instance) {
    if (ffmpeg != NULL) {
        ffmpeg->start();
    }
}
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeStop(JNIEnv *env, jobject instance,
                                                              jboolean exit) {
    if (ffmpeg != NULL) {
        ffmpeg->exitByUser = true;
        ffmpeg->release();
        delete ffmpeg;
        ffmpeg = NULL;
    }
    if (javaCall != NULL) {
        javaCall->release();
        javaCall = NULL;
    }
    if (!exit) {
        jclass jlz = env->GetObjectClass(instance);
        jmethodID jmid_stop = env->GetMethodID(jlz, "onStopComplete", "()V");
        env->CallVoidMethod(instance, jmid_stop);
    }
}
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativePause(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }

}
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeResume(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }

}
JNIEXPORT void JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeSeek(JNIEnv *env, jobject instance,
                                                              jint secds) {
    if (ffmpeg != NULL) {
        ffmpeg->seek(secds);
    }

}

JNIEXPORT jint JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeGetDuration(JNIEnv *env,
                                                                     jobject instance) {
    if (ffmpeg != NULL) {
        return ffmpeg->getDuration();
    }
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeGetVideoWidth(JNIEnv *env,
                                                                       jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->getVideoWidth();
    }

}
JNIEXPORT jint JNICALL
Java_com_h_arrow_medialib_mediaplayer_HMediaPlayer_nativeGetVideoHeidht(JNIEnv *env,
                                                                        jobject instance) {
    if (ffmpeg != NULL) {
        ffmpeg->getVideoHeight();
    }

}
}

//extern "C"
//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
//    jint result = -1;
//    javaVM = vm;
//    JNIEnv *env;
//    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
//        return -1;
//    }
//    return JNI_VERSION_1_4;
//}

