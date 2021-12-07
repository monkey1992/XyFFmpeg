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
#include "../include/libavutil/imgutils.h"

const char *LOG_TAG = (const char *) "video_player";

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
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "start play video url : %s", (char *) url);
    // 视频url
    const char *file_name = (*env)->GetStringUTFChars(env, url, JNI_FALSE);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "start play video file_name : %s", file_name);

    avdevice_register_all();

    AVFormatContext *pFormatContext = avformat_alloc_context();

    // 打开视频文件
    if (avformat_open_input(&pFormatContext, file_name, NULL, NULL) != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Couldn't open url : %s", file_name);
        return -1;
    }

    // 检索流信息
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Couldn't find stream information.");
        return -1;
    }

    // 找到第一个视频流
    int videoStream = -1, i;
    for (i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            videoStream < 0) {
            videoStream = i;
        }
    }
    if (videoStream == -1) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Didn't find a video stream.");
        return -1; // 找不到视频流，返回-1
    }

    AVCodecParameters *avCodecParameters = pFormatContext->streams[videoStream]->codecpar;

    // 找到视频流对应的解码器
    const AVCodec *pCodec = avcodec_find_decoder(avCodecParameters->codec_id);

    // 得到视频流的编解码器上下文环境的指针
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);

    if (pCodec == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Codec not found.");
        return -1; // 找不到视频解码器，返回-1
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Could not open codec.");
        return -1; // 打开解码器失败，返回-1
    }

    // 获取NativeWindow，用于渲染视频
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高值
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

    // 设置NativeWindows的Buffer大小，可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    if (avcodec_open2(pCodecCtx, pCodec, NULL)) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Could not open codec.");
        return -1;
    }

    //分配视频帧空间内存
    AVFrame *pFrame = av_frame_alloc();
    //用于渲染
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Could not allocate video frame.");
        return -1;
    }

    // 确定所需缓冲区大小并分配缓冲区内存空间
    // Buffer中的数据就是用于渲染的，且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height,
                                            1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的，故在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(
            pCodecCtx->width,
            pCodecCtx->height,
            pCodecCtx->pix_fmt,
            pCodecCtx->width,
            pCodecCtx->height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL);
    int frameFinished;
    AVPacket packet;
    while (av_read_frame(pFormatContext, &packet) >= 0) {
        // 判断Packet（音视频压缩数据）是否是视频流
        if (packet.stream_index == videoStream) {
            pCodec->decode(pCodecCtx, pFrame, &frameFinished, &packet);
//            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // 并不是解码一次就可以解码出1帧
            if (frameFinished) {

                // 锁住NativeWindow缓冲区
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGBA->data,
                          pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (uint8_t *) (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
                // 由于窗口的stride和帧的stride不同，因此需要逐行复制
                int h;
                for (int h = 0; h < videoHeight; ++h) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_free(pFrameRGBA);

    // 释放YUV图像帧
    av_free(pFrame);
    // 关闭解码器
    avcodec_free_context(&pCodecCtx);
    // 关闭视频文件
    avformat_close_input(&pFormatContext);
    return 0;
}