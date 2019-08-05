package com.h.arrow.videoeditor.activity;

import android.Manifest;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.VideoView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.google.android.material.snackbar.Snackbar;
import com.h.arrow.medialib.videoeditor.VideoEditor;
import com.h.arrow.videoeditor.R;
import com.h.arrow.videoeditor.utils.ThreadPool;
import com.h.arrow.videoeditor.utils.Utils;

import org.florescu.android.rangeseekbar.RangeSeekBar;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {


    private static final int REQUEST_TAKE_GALLERY_VIDEO = 100;
    private VideoView videoView;
    private RangeSeekBar rangeSeekBar;
    private TextView tvLeft, tvRight;
    private Runnable r;
    private ProgressDialog progressDialog;
    private Uri selectedVideoUri;
    private int stopPosition;
    private LinearLayout mainlayout;
    private String filePath;
    private int duration;
    private int choice;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final TextView uploadVideo = findViewById(R.id.uploadVideo);
        Button extractAv = findViewById(R.id.extractAV);
        Button mergeAv = findViewById(R.id.mergeAV);
        tvLeft = findViewById(R.id.tvLeft);
        tvRight = findViewById(R.id.tvRight);
        videoView = findViewById(R.id.videoView);
        rangeSeekBar = findViewById(R.id.rangeSeekBar);
        mainlayout = findViewById(R.id.mainlayout);
        progressDialog = new ProgressDialog(this);
        progressDialog.setTitle(null);
        progressDialog.setCancelable(false);
        rangeSeekBar.setEnabled(false);

        uploadVideo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (Build.VERSION.SDK_INT >= 23)
                    getPermission();
                else
                    uploadVideo();

            }
        });

        extractAv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                choice = 1;
                if (selectedVideoUri != null) {
                    if (Build.VERSION.SDK_INT >= 23)
                        getAudioPermission();
                    else
                        extractAudioVideo();
                } else
                    Snackbar.make(mainlayout, "Please upload a video", 4000).show();
            }
        });

        mergeAv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                choice = 2;
                if (selectedVideoUri != null) {
                    if (Build.VERSION.SDK_INT >= 23)
                        getAudioPermission();
                    else
                        mergeAudioVideo();
                } else
                    Snackbar.make(mainlayout, "Please upload a video", 4000).show();
            }
        });
        findViewById(R.id.cut_video).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                choice = 3;
                if (selectedVideoUri != null) {
                    if (Build.VERSION.SDK_INT >= 23)
                        getAudioPermission();
                    else
                        cutVideo();
                } else
                    Snackbar.make(mainlayout, "Please upload a video", 4000).show();
            }
        });

    }

    private void getPermission() {
        String[] params = null;
        String writeExternalStorage = Manifest.permission.WRITE_EXTERNAL_STORAGE;
        String readExternalStorage = Manifest.permission.READ_EXTERNAL_STORAGE;

        int hasWriteExternalStoragePermission = ActivityCompat.checkSelfPermission(this, writeExternalStorage);
        int hasReadExternalStoragePermission = ActivityCompat.checkSelfPermission(this, readExternalStorage);
        List<String> permissions = new ArrayList<String>();

        if (hasWriteExternalStoragePermission != PackageManager.PERMISSION_GRANTED)
            permissions.add(writeExternalStorage);
        if (hasReadExternalStoragePermission != PackageManager.PERMISSION_GRANTED)
            permissions.add(readExternalStorage);

        if (!permissions.isEmpty()) {
            params = permissions.toArray(new String[permissions.size()]);
        }
        if (params != null && params.length > 0) {
            ActivityCompat.requestPermissions(MainActivity.this,
                    params,
                    100);
        } else
            uploadVideo();
    }

    private void getAudioPermission() {
        String[] params = null;
        String recordAudio = Manifest.permission.RECORD_AUDIO;
        String modifyAudio = Manifest.permission.MODIFY_AUDIO_SETTINGS;

        int hasRecordAudioPermission = ActivityCompat.checkSelfPermission(this, recordAudio);
        int hasModifyAudioPermission = ActivityCompat.checkSelfPermission(this, modifyAudio);
        List<String> permissions = new ArrayList<String>();

        if (hasRecordAudioPermission != PackageManager.PERMISSION_GRANTED)
            permissions.add(recordAudio);
        if (hasModifyAudioPermission != PackageManager.PERMISSION_GRANTED)
            permissions.add(modifyAudio);

        if (!permissions.isEmpty()) {
            params = permissions.toArray(new String[permissions.size()]);
        }
        if (params != null && params.length > 0) {
            ActivityCompat.requestPermissions(MainActivity.this,
                    params,
                    200);
        } else {
            switch (choice) {
                case 1:
                    extractAudioVideo();
                    break;
                case 2:
                    mergeAudioVideo();
                    break;
                case 3:
                    cutVideo();
                    break;
            }

        }

    }

    /**
     * Handling response for permission request
     */
    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case 100: {

                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    uploadVideo();
                }
            }
            break;
            case 200: {

                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    extractAudioVideo();
                }
            }


        }
    }

    /**
     * Opening gallery for uploading video
     */
    private void uploadVideo() {
        try {
            Intent intent = new Intent();
            intent.setType("video/*");
            intent.setAction(Intent.ACTION_GET_CONTENT);
            startActivityForResult(Intent.createChooser(intent, "Select Video"), REQUEST_TAKE_GALLERY_VIDEO);
        } catch (Exception e) {

        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        stopPosition = videoView.getCurrentPosition(); //stopPosition is an int
        videoView.pause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        videoView.seekTo(stopPosition);
        videoView.start();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            if (requestCode == REQUEST_TAKE_GALLERY_VIDEO) {
                selectedVideoUri = data.getData();
                videoView.setVideoURI(selectedVideoUri);
                videoView.start();


                videoView.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {

                    @Override
                    public void onPrepared(MediaPlayer mp) {
                        // TODO Auto-generated method stub
                        duration = mp.getDuration() / 1000;
                        tvLeft.setText("00:00:00");

                        tvRight.setText(getTime(mp.getDuration() / 1000));
                        mp.setLooping(true);
                        rangeSeekBar.setRangeValues(0, duration);
                        rangeSeekBar.setSelectedMinValue(0);
                        rangeSeekBar.setSelectedMaxValue(duration);
                        rangeSeekBar.setEnabled(true);

                        rangeSeekBar.setOnRangeSeekBarChangeListener(new RangeSeekBar.OnRangeSeekBarChangeListener() {
                            @Override
                            public void onRangeSeekBarValuesChanged(RangeSeekBar bar, Object minValue, Object maxValue) {
                                videoView.seekTo((int) minValue * 1000);

                                tvLeft.setText(getTime((int) bar.getSelectedMinValue()));

                                tvRight.setText(getTime((int) bar.getSelectedMaxValue()));

                            }
                        });

                        final Handler handler = new Handler();
                        handler.postDelayed(r = new Runnable() {
                            @Override
                            public void run() {

                                if (videoView.getCurrentPosition() >= rangeSeekBar.getSelectedMaxValue().intValue() * 1000)
                                    videoView.seekTo(rangeSeekBar.getSelectedMinValue().intValue() * 1000);
                                handler.postDelayed(r, 1000);
                            }
                        }, 1000);

                    }
                });

//                }
            }
        }
    }

    private String getTime(int seconds) {
        int hr = seconds / 3600;
        int rem = seconds % 3600;
        int mn = rem / 60;
        int sec = rem % 60;
        return String.format("%02d", hr) + ":" + String.format("%02d", mn) + ":" + String.format("%02d", sec);
    }


    private void showUnsupportedExceptionDialog() {
        new AlertDialog.Builder(MainActivity.this)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setTitle("Not Supported")
                .setMessage("Device Not Supported")
                .setCancelable(false)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        MainActivity.this.finish();
                    }
                })
                .create()
                .show();

    }

    private void extractAudioVideo() {
        File moviesDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_MUSIC
        );

        String filePrefix = "extract_audio";
        String fileExtn = ".aac";
        final String yourRealPath = Utils.getPath(MainActivity.this, selectedVideoUri);
        File dest = new File(moviesDir, filePrefix + fileExtn);
        filePath = dest.getAbsolutePath();
        final String videoPath = filePath.replace("aac", "mp4");


        final File desc = new File(moviesDir, "merge.mp4");


        ThreadPool.getInstance().execute(new Runnable() {
            @Override
            public void run() {
                new VideoEditor().extractAudio(yourRealPath, videoPath, filePath);
            }
        });

    }

    private void mergeAudioVideo() {
        File moviesDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_MUSIC
        );

        String filePrefix = "extract_audio";
        String fileExtn = ".aac";

        File source = new File(moviesDir, filePrefix + fileExtn);
        filePath = source.getAbsolutePath();

        final String videoPath = filePath.replace("aac", "mp4");
        File desc = new File(moviesDir, "merge2.mp4");
        final String outPath = desc.getAbsolutePath();
        final String yourRealPath = Utils.getPath(MainActivity.this, selectedVideoUri);

        String filePrefix2 = "extract_audio2";
        String fileExtn2 = ".aac";
        File dest = new File(moviesDir, filePrefix2 + fileExtn2);
        final String filePath2 = dest.getAbsolutePath();

        ThreadPool.getInstance().execute(new Runnable() {
            @Override
            public void run() {
                new VideoEditor().mergeVideoAudio(videoPath, filePath, outPath);
//                new VideoEditor().testAcc(filePath,filePath2);
            }
        });

    }

    private void cutVideo() {
        File moviesDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_MUSIC
        );

        String filePrefix = "video_cut";
        String fileExtn = ".mp4";

        File source = new File(moviesDir, filePrefix + fileExtn);
        filePath = source.getAbsolutePath();
        final String yourRealPath = Utils.getPath(MainActivity.this, selectedVideoUri);
        ThreadPool.getInstance().execute(new Runnable() {
            @Override
            public void run() {
                new VideoEditor().videoCut(yourRealPath, filePath, 0, 30);
//                new VideoEditor().testAcc(filePath,filePath2);
            }
        });

    }

}
