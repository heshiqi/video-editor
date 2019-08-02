package com.h.arrow.medialib.videoeditor;

public class VideoEditor {

    static {
        System.loadLibrary("player-lib");
    }


    public native void extractAudio(String path, String videoPath,String audioPath);

}
