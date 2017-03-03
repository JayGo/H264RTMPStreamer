/*
 * edu_fudan_stormcv_util_H264RTMPStreamer.cpp
 *
 *  Created on: Jan 11, 2017
 *      Author: jliu
 */
#include "RTMPStream.h"
#include "./include/edu_fudan_stormcv_util_StreamerHelper.h"
#include "./include/convert_utils.h"

static CRTMPStream rtmpStream;

/**
 * ==============================================
 * JNI interface
 * ==============================================
 */

JNIEXPORT jboolean JNICALL Java_edu_fudan_stormcv_util_StreamerHelper_sendFirstFrame(
		JNIEnv *env, jobject obj, jbyteArray jFrameBytes) {
	int frameBytesLength = 0;
	unsigned char* frameBytes = (unsigned char*) jbyteArrayTochar(env,
			jFrameBytes, frameBytesLength);
	return rtmpStream.sendFirstFrame(frameBytes, frameBytesLength);
}

JNIEXPORT jboolean JNICALL Java_edu_fudan_stormcv_util_StreamerHelper_sendNormalFrame(
		JNIEnv *env, jobject obj, jbyteArray jFrameBytes) {
	int frameBytesLength = 0;
	unsigned char* frameBytes = (unsigned char*) jbyteArrayTochar(env,
			jFrameBytes, frameBytesLength);
	return rtmpStream.sendNormalFrame(frameBytes, frameBytesLength);
}

JNIEXPORT jboolean JNICALL Java_edu_fudan_stormcv_util_StreamerHelper_connectToRtmp
  (JNIEnv *env, jobject obj, jstring jRtmpAddr) {
	char *rtmpAddr = jstringToChar(env, jRtmpAddr);
	return rtmpStream.Connect(rtmpAddr);
}

