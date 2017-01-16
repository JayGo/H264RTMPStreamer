/*
 * Main.cpp
 *
 *  Created on: Dec 28, 2016
 *      Author: jliu
 */

#include "RTMPStream.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
//	printf("test\n");
//	cout << "test" <<endl;
	CRTMPStream *mCRTMPStream = new CRTMPStream();

	string rtmpAddress = "rtmp://10.134.142.100:1935/live1/test";
	string videoDir = "/home/jliu/H264TestFrames/";
	string h264FileName = "/home/jliu/H264TestFrames/test";



	if (mCRTMPStream->Connect(rtmpAddress.c_str()) > 0) {
		cout << "Connect to " << rtmpAddress << " successfully!" << endl;
	} else {
		cout << "Connect to " << rtmpAddress << " failed!" << endl;
	}

//	while(1) {
//		 mCRTMPStream->SendH264File(h264FileName.c_str());
//	}



	while(1) {
		mCRTMPStream->SendH264Frames(videoDir.c_str());
	}



	cout << "program end!" << endl;
	return -1;
}

