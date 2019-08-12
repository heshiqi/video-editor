package com.h.arrow.medialib.mediaplayer.view;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.h.arrow.medialib.mediaplayer.listener.HOnGlSurfaceViewOncreateListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnRenderRefreshListener;

public class HGLSurfaceView extends GLSurfaceView {

    private HGLRender render;
    private HOnGlSurfaceViewOncreateListener onGlSurfaceViewOncreateListener;


    public HGLSurfaceView(Context context) {
        this(context, null);
    }

    public HGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        render = new HGLRender(context);
        //设置egl版本为2.0
        setEGLContextClientVersion(2);
        //设置render
        setRenderer(render);
        //设置为手动刷新模式
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        render.setOnRenderRefreshListener(new HOnRenderRefreshListener() {
            @Override
            public void onRefresh() {
                requestRender();
            }
        });
    }

    public void setOnGlSurfaceViewOncreateListener(HOnGlSurfaceViewOncreateListener onGlSurfaceViewOncreateListener) {
        if (render != null) {
            render.setOnGLSurfaceViewCreateListener(onGlSurfaceViewOncreateListener);
        }
    }

    public void setCodecType(int type) {
        if (render != null) {
            render.setCodecType(type);
        }
    }


    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v) {
        if (render != null) {
            render.setFrameData(w, h, y, u, v);
            requestRender();
        }
    }

    public void cutVideoImg() {
        if (render != null) {
            render.cutVideoImg();
            requestRender();
        }
    }

}
