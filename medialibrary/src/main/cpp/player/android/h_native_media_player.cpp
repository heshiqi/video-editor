//
// Created by 何士奇 on 2019-08-12.
//

#include <jni.h>
#include <cwchar>
#include "../../android_log.h"

extern "C" {
#include <libavcodec/jni.h>
}

const char *JAVE_CLASS_NAME = "com/h/arrow/medialib/mediaplayer/HNativeMediaPlayer";

static JavaVM *javaVM = NULL;
struct fields_t {
    jfieldID context;
    jmethodID post_event;
};

static fields_t fields;

void _init(JNIEnv *env) {

    jclass clazz = env->FindClass(JAVE_CLASS_NAME);
    if (clazz == NULL) {
        return;
    }
    fields.context = env->GetFieldID(clazz, "mNativeContext", "J");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative","(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        return;
    }

    env->DeleteLocalRef(clazz);
}

void _setup(JNIEnv *env, jobject thiz, jobject mediaplayer_this) {

//    CainMediaPlayer *mp = new CainMediaPlayer();
//    if (mp == NULL) {
//        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
//        return;
//    }
//
//    // 这里似乎存在问题
//    // init CainMediaPlayer
//    mp->init();
//
//    // create new listener and give it to MediaPlayer
//    JNIMediaPlayerListener *listener = new JNIMediaPlayerListener(env, thiz, mediaplayer_this);
//    mp->setListener(listener);
//
//    // Stow our new C++ MediaPlayer in an opaque field in the Java object.
//    setMediaPlayer(env, thiz, (long)mp);
}


static const JNINativeMethod gMethods[] = {
        {"native_init", "()V", (void *) _init},
        {"native_setup", "(Ljava/lang/Object;)V", (void *) _setup},
};


// 注册HNativeMediaPlayer的Native方法
static int register_com_h_arrow_medialib_mediaplayer_HNativeMediaPlayer(JNIEnv *env) {
    int numMethods = (sizeof(gMethods) / sizeof((gMethods)[0]));
    jclass clazz = env->FindClass(JAVE_CLASS_NAME);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", JAVE_CLASS_NAME);
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("Native registration unable to find class '%s'", JAVE_CLASS_NAME);
        return JNI_ERR;
    }
    env->DeleteLocalRef(clazz);

    return JNI_OK;
}


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    av_jni_set_java_vm(vm, NULL);
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    if (register_com_h_arrow_medialib_mediaplayer_HNativeMediaPlayer(env) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}