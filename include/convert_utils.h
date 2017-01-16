/*
 * convertutils.h
 *
 *  Created on: Dec 16, 2016
 *      Author: jliu
 */

#ifndef CONVERTUTILS_H_
#define CONVERTUTILS_H_

jstring charTojstring(JNIEnv* env, const char* pat);

char* jstringToChar(JNIEnv* env, jstring jstr);

jbyteArray charTojbyteArray(JNIEnv *env, char* chs, int length);

char* jbyteArrayTochar(JNIEnv *env, jbyteArray jbyte_array, int& length);

jintArray intArrayTojintArray(JNIEnv *env, int* ints, int length);

#endif /* CONVERTUTILS_H_ */
