package com.h.arrow.medialib.mediaplayer;

import android.graphics.Bitmap;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.h.arrow.medialib.mediaplayer.listener.HOnCompleteListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnCutVideoImgListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnErrorListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnGlSurfaceViewOncreateListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnInfoListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnLoadListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnPreparedListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnStopListener;
import com.h.arrow.medialib.mediaplayer.listener.HStatus;
import com.h.arrow.medialib.mediaplayer.view.HGLSurfaceView;

import java.nio.ByteBuffer;

public class HMediaPlayer {

    static {
//        System.loadLibrary("avcodec-");
//        System.loadLibrary("avfilter-");
//        System.loadLibrary("avformat-");
//        System.loadLibrary("avutil-");
//        System.loadLibrary("postproc-");
//        System.loadLibrary("swresample-");
//        System.loadLibrary("swscale-");
//        System.loadLibrary("x264-");

        System.loadLibrary("player-lib");
    }

    //视频源路径
    private String dataSource;

    //硬解码mime
    private MediaFormat mediaFormat;

    //解码器
    private MediaCodec mediaCodec;

    private Surface mSurface;

    private HGLSurfaceView hglSurfaceView;

    private MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();


    /**
     * 准备好时的回调
     */
    private HOnPreparedListener onPreparedListener;
    /**
     * 错误时的回调
     */
    private HOnErrorListener onErrorListener;
    /**
     * 加载回调
     */
    private HOnLoadListener onLoadListener;
    /**
     * 更新时间回调
     */
    private HOnInfoListener onInfoListener;
    /**
     * 播放完成回调
     */
    private HOnCompleteListener onCompleteListener;
    /**
     * 视频截图回调
     */
    private HOnCutVideoImgListener onCutVideoImgListener;
    /**
     * 停止完成回调
     */
    private HOnStopListener onStopListener;
    /**
     * 是否已经准备好
     */
    private boolean parpared = false;

    /**
     * 时长实体类
     */
    private TimeBean timeBean;

    /**
     * 上一次播放时间
     */
    private int lastCurrTime = 0;

    /**
     * 是否只有音频（只播放音频流）
     */
    private boolean isOnlyMusic = false;

    private boolean isOnlySoft = false;

    public HMediaPlayer() {
        timeBean = new TimeBean();
    }

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setOnlyMusic(boolean onlyMusic) {
        isOnlyMusic = onlyMusic;
    }

    private void setSurface(Surface surface) {
        this.mSurface = surface;
    }

    public void setGlSurfaceView(HGLSurfaceView glSurfaceView) {
        this.hglSurfaceView = glSurfaceView;
        hglSurfaceView.setOnGlSurfaceViewOncreateListener(new HOnGlSurfaceViewOncreateListener() {

            @Override
            public void onGLSurfaceViewCreate(Surface surface) {
                if (mSurface == null) {
                    setSurface(surface);
                }
                if (parpared && !TextUtils.isDigitsOnly(dataSource)) {
                    nativePrepare(dataSource, isOnlyMusic);
                }
            }

            @Override
            public void onCutVideoImg(Bitmap bitmap) {
                if (onCutVideoImgListener != null) {
                    onCutVideoImgListener.onCutVideoImg(bitmap);
                }
            }
        });
    }

    private native void nativePrepare(String url, boolean isOnlyMusic);

    private native void nativeStart();

    private native void nativeStop(boolean exit);

    private native void nativePause();

    private native void nativeResume();

    private native void nativeSeek(int secds);

    private native int nativeGetDuration();

    private native int nativeGetVideoWidth();

    private native int nativeGetVideoHeidht();

    public int getDuration() {
        return nativeGetDuration();
    }

    public void setOnPreparedListener(HOnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    public void setOnErrorListener(HOnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void prepared() {
        if (TextUtils.isEmpty(dataSource)) {
            onError(HStatus.WL_STATUS_DATASOURCE_NULL, "datasource is null");
            return;
        }
        parpared = true;
        if (isOnlyMusic) {
            nativePrepare(dataSource, isOnlyMusic);
        } else {
            if (mSurface != null) {
                nativePrepare(dataSource, isOnlyMusic);
            }
        }
    }

    public void start() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (TextUtils.isEmpty(dataSource)) {
                    onError(HStatus.WL_STATUS_DATASOURCE_NULL, "datasource is null");
                    return;
                }
                if (!isOnlyMusic) {
                    if (mSurface == null) {
                        onError(HStatus.WL_STATUS_SURFACE_NULL, "surface is null");
                        return;
                    }
                }

                if (timeBean == null) {
                    timeBean = new TimeBean();
                }
                nativeStart();
            }
        }).start();
    }

    public void stop(final boolean exit) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                nativeStop(exit);
                if (mediaCodec != null) {
                    try {
                        mediaCodec.flush();
                        mediaCodec.stop();
                        mediaCodec.release();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    mediaCodec = null;
                    mediaFormat = null;
                }
                if (hglSurfaceView != null) {
                    hglSurfaceView.setCodecType(-1);
                    hglSurfaceView.requestRender();
                }

            }
        }).start();
    }

    public void pause() {
        nativePause();

    }

    public void resume() {
        nativeResume();
    }

    public void seek(final int secds) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                nativeSeek(secds);
                lastCurrTime = secds;
            }
        }).start();
    }

    public void setOnlySoft(boolean soft) {
        this.isOnlySoft = soft;
    }

    public boolean isOnlySoft() {
        return isOnlySoft;
    }

    private void onLoad(boolean load) {
        if (onLoadListener != null) {
            onLoadListener.onLoad(load);
        }
    }


    private void onParpared() {
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }

    private void onError(int code, String msg) {
        if (onErrorListener != null) {
            onErrorListener.onError(code, msg);
        }
        stop(true);
    }

    public void mediacodecInit(int mimetype, int width, int height, byte[] csd0, byte[] csd1) {
        if (mSurface != null) {
            try {
                hglSurfaceView.setCodecType(1);
                String mtype = getMimeType(mimetype);
                mediaFormat = MediaFormat.createVideoFormat(mtype, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_WIDTH, width);
                mediaFormat.setInteger(MediaFormat.KEY_HEIGHT, height);
                mediaFormat.setLong(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd1));
                Log.d("ywl5320", mediaFormat.toString());
                mediaCodec = MediaCodec.createDecoderByType(mtype);
                mediaCodec.configure(mediaFormat, mSurface, null, 0);
                mediaCodec.start();
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            if (onErrorListener != null) {
                onErrorListener.onError(HStatus.WL_STATUS_SURFACE_NULL, "surface is null");
            }
        }
    }


    public void mediacodecDecode(byte[] bytes, int size, int pts) {
        if (bytes != null && mediaCodec != null && info != null) {
            try {
                int inputBufferIndex = mediaCodec.dequeueInputBuffer(10);
                if (inputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(bytes);
                    mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, pts, 0);
                }
                int index = mediaCodec.dequeueOutputBuffer(info, 10);
                while (index >= 0) {
                    mediaCodec.releaseOutputBuffer(index, true);
                    index = mediaCodec.dequeueOutputBuffer(info, 10);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void setOnLoadListener(HOnLoadListener onLoadListener) {
        this.onLoadListener = onLoadListener;
    }

    private String getMimeType(int type) {
        if (type == 1) {
            return "video/avc";
        } else if (type == 2) {
            return "video/hevc";
        } else if (type == 3) {
            return "video/mp4v-es";
        } else if (type == 4) {
            return "video/x-ms-wmv";
        }
        return "";
    }

    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v) {
        if (hglSurfaceView != null) {
            Log.d("hh", "setFrameData");
            hglSurfaceView.setCodecType(0);
            hglSurfaceView.setFrameData(w, h, y, u, v);
        }
    }

    public void setOnInfoListener(HOnInfoListener onInfoListener) {
        this.onInfoListener = onInfoListener;
    }

    public void setVideoInfo(int currt_secd, int total_secd) {
        if (onInfoListener != null && timeBean != null) {
            if (currt_secd < lastCurrTime) {
                currt_secd = lastCurrTime;
            }
            timeBean.setCurrt_secds(currt_secd);
            timeBean.setTotal_secds(total_secd);
            onInfoListener.onInfo(timeBean);
            lastCurrTime = currt_secd;
        }
    }

    public void setOnCompleteListener(HOnCompleteListener onCompleteListener) {
        this.onCompleteListener = onCompleteListener;
    }


    public void videoComplete() {
        if (onCompleteListener != null) {
            setVideoInfo(nativeGetDuration(), nativeGetDuration());
            timeBean = null;
            onCompleteListener.onComplete();
        }
    }

    public void setOnCutVideoImgListener(HOnCutVideoImgListener onCutVideoImgListener) {
        this.onCutVideoImgListener = onCutVideoImgListener;
    }

    public void cutVideoImg() {
        if (hglSurfaceView != null) {
            hglSurfaceView.cutVideoImg();
        }
    }

    public void setOnStopListener(HOnStopListener onStopListener) {
        this.onStopListener = onStopListener;
    }

    public void onStopComplete() {
        if (onStopListener != null) {
            onStopListener.onStop();
        }
    }
}
