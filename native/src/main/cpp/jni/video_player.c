//
// Created by xiaoyu on 2021/11/24.
//

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include "video_player.h"
#include "../include/libavcodec/avcodec.h"
#include "../include/libavformat/avformat.h"
#include "../include/libswscale/swscale.h"
#include "../include/libavdevice/avdevice.h"
#include "../include/libavutil/avutil.h"

#define LOG_TAG 'video_player'

int printLog(int prio, const char* fmt, ...) {
    __android_log_print(prio, LOG_TAG, "%s", fmt);
}

/*
* Class:     com_xy_ffmpeg_VideoPlayer
* Method:    test
* Signature: ()I
*/
JNIEXPORT jstring JNICALL Java_com_xy_ffmpeg_VideoPlayer_test
        (JNIEnv *env, jobject object) {
//    return (*env)->NewStringUTF((JNIEnv *) "Hello VideoPlayer", "123");
    return (*env)->NewStringUTF(env, "123");
}

/*
 * Class:     com_xy_ffmpeg_VideoPlayer
 * Method:    playVideo
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_com_xy_ffmpeg_VideoPlayer_playVideo
        (JNIEnv *env, jobject object, jstring url, jobject surface) {
    printLog(ANDROID_LOG_DEBUG, "start play video... url");
    // 视频url
    const char * file_name = (*env)->GetStringUTFChars(env, url, JNI_FALSE);
    printLog(ANDROID_LOG_DEBUG, "start play video... url %s", file_name);

    avdevice_register_all();

    AVFormatContext * pFormatContext  = avformat_alloc_context();
    // 打开视频文件
    if (avformat_open_input(&pFormatContext, file_name, NULL, NULL) != 0) {
        printLog(ANDROID_LOG_DEBUG, "Couldn't open file:%s\n", file_name);
        return -1;
    }

    // 检索信息流
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        printLog(ANDROID_LOG_DEBUG, "Couldn't find stream information.");
        return -1;
    }

    // 找到第一个视频流
    int videoStream = -1, i;
    for (i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0) {
            videoStream = i;
        }
    }

    return -1;
}