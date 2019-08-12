package com.h.arrow.medialib.mediaplayer;

import android.os.Handler;
import android.os.Looper;

import java.lang.ref.WeakReference;

public class HNativeMediaPlayer extends HBaseMediaPlayer {

    static {
        System.loadLibrary("player-lib");
        native_init();
    }

    private H mH;

    public HNativeMediaPlayer() {
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mH = new H(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mH = new H(this, looper);
        } else {
            mH = null;
        }
        /* Native setup requires a weak reference to our object.
         * It's easier to create it here than in C++.
         */
        native_setup(new WeakReference<HNativeMediaPlayer>(this));
    }

    private static native void native_init();
    private static native void native_setup(Object mediaplayer_this);


    private class H extends Handler {
        private HNativeMediaPlayer mMediaPlayer;
        public H(HNativeMediaPlayer mp, Looper looper) {
            super(looper);
            mMediaPlayer = mp;
        }
    }

}
