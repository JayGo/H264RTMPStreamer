/*
 * convertutils.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: jliu
 */

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "./include/convert_utils.h"

using namespace std;

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

jbyteArray charTojbyteArray(JNIEnv *env, char* chs, int length) {
	jbyte *by = (jbyte*)chs;
	jbyteArray jarray = env->NewByteArray(length);
	env->SetByteArrayRegion(jarray, 0, length, by);
	return jarray;
}

char* jbyteArrayTochar(JNIEnv *env, jbyteArray jbyte_array, int& length) {
	jbyte* jbyte_ptr = (jbyte*)env->GetByteArrayElements(jbyte_array, 0);
	length = env->GetArrayLength(jbyte_array);
	return (char*)jbyte_ptr;
}

jintArray intArrayTojintArray(JNIEnv *env, int* ints, int length) {
	jint *jinteger = (jint*)ints;
	jintArray jarray = env->NewIntArray(length);
	env->SetIntArrayRegion(jarray, 0, length, jinteger);
	return jarray;
}

jstring charTojstring(JNIEnv* env, const char* pat) {
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*) pat);
    jstring encoding = (env)->NewStringUTF("GB2312");
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

