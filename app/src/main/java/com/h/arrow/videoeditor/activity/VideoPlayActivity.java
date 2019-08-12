package com.h.arrow.videoeditor.activity;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.h.arrow.medialib.mediaplayer.HMediaPlayer;
import com.h.arrow.medialib.mediaplayer.TimeBean;
import com.h.arrow.medialib.mediaplayer.listener.HOnCompleteListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnCutVideoImgListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnErrorListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnInfoListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnLoadListener;
import com.h.arrow.medialib.mediaplayer.listener.HOnPreparedListener;
import com.h.arrow.medialib.mediaplayer.view.HGLSurfaceView;
import com.h.arrow.medialib.utils.TimeUtil;
import com.h.arrow.videoeditor.R;


public class VideoPlayActivity extends AppCompatActivity {

    private HGLSurfaceView surfaceview;
    private HMediaPlayer wlPlayer;
    private ProgressBar progressBar;
    private TextView tvTime;
    private ImageView ivPause;
    private SeekBar seekBar;
    private String pathurl;
    private LinearLayout lyAction;
    private ImageView ivCutImg;
    private ImageView ivShowImg;
    private boolean ispause = false;
    private int position;
    private boolean isSeek = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_video_play);
        surfaceview = findViewById(R.id.surfaceview);
        progressBar = findViewById(R.id.pb_loading);
        ivPause = findViewById(R.id.iv_pause);
        tvTime = findViewById(R.id.tv_time);
        seekBar = findViewById(R.id.seekbar);
        lyAction = findViewById(R.id.ly_action);
        ivCutImg = findViewById(R.id.iv_cutimg);
        ivShowImg = findViewById(R.id.iv_show_img);

        wlPlayer = new HMediaPlayer();
        wlPlayer.setOnlyMusic(false);

        pathurl = getIntent().getExtras().getString("url");
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                position = wlPlayer.getDuration() * progress / 100;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isSeek = true;
                wlPlayer.seek(position);
            }
        });
        wlPlayer.setDataSource(pathurl);
        wlPlayer.setOnlySoft(false);
        wlPlayer.setGlSurfaceView(surfaceview);
        wlPlayer.setOnErrorListener(new HOnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.d("hh", "code:" + code + ",msg:" + msg);
                Message message = Message.obtain();
                message.obj = msg;
                message.what = 3;
                handler.sendMessage(message);
            }
        });

        wlPlayer.setOnPreparedListener(new HOnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d("hh", "starting......");
                wlPlayer.start();
            }
        });

        wlPlayer.setOnLoadListener(new HOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                Log.d("hh", "loading ......>>>   " + load);
                Message message = Message.obtain();
                message.what = 1;
                message.obj = load;
                handler.sendMessage(message);
            }
        });

        wlPlayer.setOnInfoListener(new HOnInfoListener() {
            @Override
            public void onInfo(TimeBean wlTimeBean) {
                Message message = Message.obtain();
                message.what = 2;
                message.obj = wlTimeBean;
                Log.d("hh", "nowTime is " + wlTimeBean.getCurrt_secds());
                handler.sendMessage(message);
            }
        });

        wlPlayer.setOnCompleteListener(new HOnCompleteListener() {
            @Override
            public void onComplete() {
                Log.d("hh", "complete .....................");
                wlPlayer.stop(true);
            }
        });

        wlPlayer.setOnCutVideoImgListener(new HOnCutVideoImgListener() {
            @Override
            public void onCutVideoImg(Bitmap bitmap) {
                Message message = Message.obtain();
                message.what = 4;
                message.obj = bitmap;
                handler.sendMessage(message);
            }
        });

        wlPlayer.prepared();
    }


    @SuppressLint("HandlerLeak")
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 1) {
                boolean load = (boolean) msg.obj;
                if (load) {
                    progressBar.setVisibility(View.VISIBLE);
                } else {
                    if (lyAction.getVisibility() != View.VISIBLE) {
                        lyAction.setVisibility(View.VISIBLE);
                        ivCutImg.setVisibility(View.VISIBLE);
                    }
                    progressBar.setVisibility(View.GONE);
                }
            } else if (msg.what == 2) {
                TimeBean wlTimeBean = (TimeBean) msg.obj;
                if (wlTimeBean.getTotal_secds() > 0) {
                    seekBar.setVisibility(View.VISIBLE);
                    if (isSeek) {
                        seekBar.setProgress(position * 100 / wlTimeBean.getTotal_secds());
                        isSeek = false;
                    } else {
                        seekBar.setProgress(wlTimeBean.getCurrt_secds() * 100 / wlTimeBean.getTotal_secds());
                    }
                    tvTime.setText(TimeUtil.secdsToDateFormat(wlTimeBean.getTotal_secds()) + "/" + TimeUtil.secdsToDateFormat(wlTimeBean.getCurrt_secds()));
                } else {
                    seekBar.setVisibility(View.GONE);
                    tvTime.setText(TimeUtil.secdsToDateFormat(wlTimeBean.getCurrt_secds()));
                }
            } else if (msg.what == 3) {
                String err = (String) msg.obj;
                Toast.makeText(VideoPlayActivity.this, err, Toast.LENGTH_SHORT).show();
            } else if (msg.what == 4) {
                Bitmap bitmap = (Bitmap) msg.obj;
                if (bitmap != null) {
                    ivShowImg.setVisibility(View.VISIBLE);
                    ivShowImg.setImageBitmap(bitmap);
                }
            }
        }
    };

    @Override
    public void onBackPressed() {
        if (wlPlayer != null) {
            wlPlayer.stop(true);
        }
        this.finish();
    }

    public void pause(View view) {
        if (wlPlayer != null) {
            ispause = !ispause;
            if (ispause) {
                wlPlayer.pause();
                ivPause.setImageResource(R.mipmap.ic_play_play);
            } else {
                wlPlayer.resume();
                ivPause.setImageResource(R.mipmap.ic_play_pause);
            }
        }

    }

    public void cutImg(View view) {
        if (wlPlayer != null) {
            wlPlayer.cutVideoImg();
        }
    }


    public static void startActivity(Context context, Class clz, Bundle bundle) {
        Intent intent = new Intent(context, clz);
        intent.putExtras(bundle);
        context.startActivity(intent);

    }
}
