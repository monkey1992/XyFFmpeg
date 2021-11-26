//
// Created by xiaoyu on 2021/11/24.
//

#include <jni.h>
#include "video_player.h"

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

}