package com.h.arrow.medialib.videoeditor;

public class VideoEditor {

    static {
        System.loadLibrary("player-lib");
    }


    public native void extractAudio(String path, String videoPath,String audioPath);


    public native void mergeVideoAudio(String videoPath,String audioPath,String outputPath);

    public native void videoCut(String inFilePath,String outFilePath,int start,int end);

    public native void testAcc(String infile,String outFile);
}
