package com.h.arrow.medialib.mediaplayer.listener;

import android.graphics.Bitmap;
import android.view.Surface;

/**
 * Created by hlwky001 on 2017/12/18.
 */

public interface HOnGlSurfaceViewOncreateListener {

    void onGLSurfaceViewCreate(Surface surface);

    void onCutVideoImg(Bitmap bitmap);

}
