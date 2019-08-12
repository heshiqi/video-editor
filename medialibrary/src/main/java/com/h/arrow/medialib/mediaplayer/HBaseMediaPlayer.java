package com.h.arrow.medialib.mediaplayer;

import android.content.Context;
import android.net.Uri;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import java.io.FileDescriptor;
import java.io.IOException;
import java.util.Map;

public class HBaseMediaPlayer implements IMediaPlayer {
    @Override
    public void setDisplay(SurfaceHolder sh) {

    }

    @Override
    public void setSurface(Surface surface) {

    }

    @Override
    public void setDataSource(@NonNull Context context, @NonNull Uri uri) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {

    }

    @Override
    public void setDataSource(@NonNull Context context, @NonNull Uri uri, Map<String, String> headers) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {

    }

    @Override
    public void setDataSource(@NonNull String path) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {

    }

    @Override
    public void setDataSource(@NonNull String path, Map<String, String> headers) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {

    }

    @Override
    public void setDataSource(FileDescriptor fd) throws IOException, IllegalArgumentException, IllegalStateException {

    }

    @Override
    public void setDataSource(FileDescriptor fd, long offset, long length) throws IOException, IllegalArgumentException, IllegalStateException {

    }

    @Override
    public void prepare() throws IOException, IllegalStateException {

    }

    @Override
    public void prepareAsync() throws IllegalStateException {

    }

    @Override
    public void start() throws IllegalStateException {

    }

    @Override
    public void stop() throws IllegalStateException {

    }

    @Override
    public void pause() throws IllegalStateException {

    }

    @Override
    public void resume() throws IllegalStateException {

    }

    @Override
    public void setWakeMode(Context context, int mode) {

    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {

    }

    @Override
    public int getRotate() {
        return 0;
    }

    @Override
    public int getVideoWidth() {
        return 0;
    }

    @Override
    public int getVideoHeight() {
        return 0;
    }

    @Override
    public boolean isPlaying() {
        return false;
    }

    @Override
    public void seekTo(float msec) throws IllegalStateException {

    }

    @Override
    public long getCurrentPosition() {
        return 0;
    }

    @Override
    public long getDuration() {
        return 0;
    }

    @Override
    public void release() {

    }

    @Override
    public void reset() {

    }

    @Override
    public void setAudioStreamType(int streamtype) {

    }

    @Override
    public void setLooping(boolean looping) {

    }

    @Override
    public boolean isLooping() {
        return false;
    }

    @Override
    public void setVolume(float leftVolume, float rightVolume) {

    }

    @Override
    public void setAudioSessionId(int sessionId) throws IllegalArgumentException, IllegalStateException {

    }

    @Override
    public int getAudioSessionId() {
        return 0;
    }

    @Override
    public void setMute(boolean mute) {

    }

    @Override
    public void setRate(float rate) {

    }

    @Override
    public void setPitch(float pitch) {

    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {

    }

    @Override
    public void setOnCompletionListener(OnCompletionListener listener) {

    }

    @Override
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {

    }

    @Override
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {

    }

    @Override
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener listener) {

    }

    @Override
    public void setOnTimedTextListener(OnTimedTextListener listener) {

    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {

    }

    @Override
    public void setOnInfoListener(OnInfoListener listener) {

    }
}
