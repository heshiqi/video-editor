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

/**
 * java 类路径
 */
const char *CLASS_NAME = "com/h/arrow/medialib/mediaplayer/HMediaPlayer";

_JavaVM *javaVM = NULL;
JavaCall *javaCall = NULL;
HFFmpeg *ffmpeg = NULL;


void HMediaPlayer_nativePrepare(JNIEnv *env, jobject instance, jstring url_, jboolean isOnlyMusic) {
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

extern "C" {
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

static const JNINativeMethod gMethods[] = {
        {"nativePrepare", "(Ljava/lang/String;Z)V", (void *) HMediaPlayer_nativePrepare},
};

// 注册HMediaPlayer的Native方法
static int register_com_h_arrow_medialib_mediaplayer_HMediaPlayer(JNIEnv *env) {
    int numMethods = (sizeof(gMethods) / sizeof((gMethods)[0]));
    jclass clazz = env->FindClass(CLASS_NAME);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", CLASS_NAME);
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("Native registration unable to find class '%s'", CLASS_NAME);
        return JNI_ERR;
    }
    env->DeleteLocalRef(clazz);
    return JNI_OK;
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    if (register_com_h_arrow_medialib_mediaplayer_HMediaPlayer(env) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

