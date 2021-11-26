package com.xy.ffmpeg;

public class VideoPlayer {

    static {
        System.loadLibrary("xyffmpeg");
    }

    public native String test();

    public native int playVideo(String url, Object surface);
}
