/******************************************************************** 
 filename:   RTMPStream.cpp
 created:    2013-04-3
 author:     firehood
 purpose:    发送H264视频到RTMP Server，使用libRtmp库
 *********************************************************************/
#include "RTMPStream.h"
#include "SpsDecode.h"

#include <iostream>
#include <vector>
#include <dirent.h>

using namespace std;

#ifdef WIN32
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"winmm.lib")
#endif

enum {
	FLV_CODECID_H264 = 7,
};

int InitSockets() {
#ifdef WIN32
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else
	return TRUE;
#endif
}

inline void CleanupSockets() {
#ifdef WIN32
	WSACleanup();
#endif
}



char * put_byte(char *output, uint8_t nVal) {
	output[0] = nVal;
	return output + 1;
}
char * put_be16(char *output, uint16_t nVal) {
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;
	return output + 2;
}
char * put_be24(char *output, uint32_t nVal) {
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;
	return output + 3;
}
char * put_be32(char *output, uint32_t nVal) {
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;
	return output + 4;
}
char * put_be64(char *output, uint64_t nVal) {
	output = put_be32(output, nVal >> 32);
	output = put_be32(output, nVal);
	return output;
}
char * put_amf_string(char *c, const char *str) {
	uint16_t len = strlen(str);
	c = put_be16(c, len);
	memcpy(c, str, len);
	return c + len;
}
char * put_amf_double(char *c, double d) {
	*c++ = AMF_NUMBER; /* type: Number */
	{
		unsigned char *ci, *co;
		ci = (unsigned char *) &d;
		co = (unsigned char *) c;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
	return c + 8;
}

CRTMPStream::CRTMPStream(void) :
		m_pRtmp(NULL), m_nFileBufSize(0), m_nCurPos(0), tick(0), frameRate(40), lastTime(-1) {
	m_pFileBuf = new unsigned char[FILEBUFSIZE];
	memset(m_pFileBuf, 0, FILEBUFSIZE);
	InitSockets();
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
}

CRTMPStream::~CRTMPStream(void) {
	Close();
#ifdef WIN32
	WSACleanup();
#endif
	delete[] m_pFileBuf;
}

long CRTMPStream::getCurrentTime() {
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

bool CRTMPStream::Connect(const char* url) {
	if (RTMP_SetupURL(m_pRtmp, (char*) url) == FALSE) {
		RTMP_Free(m_pRtmp);
		return FALSE;
	}
	RTMP_EnableWrite(m_pRtmp);
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE) {
		RTMP_Free(m_pRtmp);
		return FALSE;
	}
	if (RTMP_ConnectStream(m_pRtmp, 0) < 0) {
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		return FALSE;
	}
	return TRUE;
}

void CRTMPStream::Close() {
	if (m_pRtmp) {
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
}

int CRTMPStream::SendPacket(unsigned int nPacketType, unsigned char *data,
		unsigned int size, unsigned int nTimestamp) {
	if (m_pRtmp == NULL) {
		return FALSE;
	}

	RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	RTMPPacket_Alloc(&packet, size);

	packet.m_packetType = nPacketType;
	packet.m_nChannel = 0x04;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nTimeStamp = nTimestamp;
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;
	packet.m_nBodySize = size;
	memcpy(packet.m_body, data, size);

	int nRet = RTMP_SendPacket(m_pRtmp, &packet, 0);

	RTMPPacket_Free(&packet);

	return nRet;
}

bool CRTMPStream::SendMetadata(LPRTMPMetadata lpMetaData) {
	if (lpMetaData == NULL) {
		return false;
	}
	char body[1024] = { 0 };
	;

	char * p = (char *) body;
	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "@setDataFrame");

	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "onMetaData");

	p = put_byte(p, AMF_OBJECT);
	p = put_amf_string(p, "copyright");
	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "firehood");

	p = put_amf_string(p, "width");
	p = put_amf_double(p, lpMetaData->nWidth);

	p = put_amf_string(p, "height");
	p = put_amf_double(p, lpMetaData->nHeight);

	p = put_amf_string(p, "framerate");
	p = put_amf_double(p, lpMetaData->nFrameRate);

	p = put_amf_string(p, "videocodecid");
	p = put_amf_double(p, FLV_CODECID_H264);

	p = put_amf_string(p, "");
	p = put_byte(p, AMF_OBJECT_END);

	int index = p - body;

	SendPacket(RTMP_PACKET_TYPE_INFO, (unsigned char*) body, p - body, 0);

	int i = 0;
	body[i++] = 0x17; // 1:keyframe  7:AVC
	body[i++] = 0x00; // AVC sequence header

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00; // fill in 0;

	// AVCDecoderConfigurationRecord.
	body[i++] = 0x01; // configurationVersion
	body[i++] = lpMetaData->Sps[1]; // AVCProfileIndication
	body[i++] = lpMetaData->Sps[2]; // profile_compatibility
	body[i++] = lpMetaData->Sps[3]; // AVCLevelIndication 
	body[i++] = 0xff; // lengthSizeMinusOne

	// sps nums
	body[i++] = 0xE1; //&0x1f
	// sps data length
	body[i++] = lpMetaData->nSpsLen >> 8;
	body[i++] = lpMetaData->nSpsLen & 0xff;
	// sps data
	memcpy(&body[i], lpMetaData->Sps, lpMetaData->nSpsLen);
	i = i + lpMetaData->nSpsLen;

	// pps nums
	body[i++] = 0x01; //&0x1f
	// pps data length 
	body[i++] = lpMetaData->nPpsLen >> 8;
	body[i++] = lpMetaData->nPpsLen & 0xff;
	// sps data
	memcpy(&body[i], lpMetaData->Pps, lpMetaData->nPpsLen);
	i = i + lpMetaData->nPpsLen;

	return SendPacket(RTMP_PACKET_TYPE_VIDEO, (unsigned char*) body, i, 0);

}

bool CRTMPStream::SendH264Packet(unsigned char *data, unsigned int size,
		bool bIsKeyFrame, unsigned int nTimeStamp) {
	if (data == NULL && size < 11) {
		return false;
	}

	unsigned char *body = new unsigned char[size + 9];

	int i = 0;
	if (bIsKeyFrame) {
		body[i++] = 0x17;	// 1:Iframe  7:AVC
	} else {
		body[i++] = 0x27;	// 2:Pframe  7:AVC
	}
	body[i++] = 0x01;	// AVC NALU
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	// NALU size
	body[i++] = size >> 24;
	body[i++] = size >> 16;
	body[i++] = size >> 8;
	body[i++] = size & 0xff;
	;

	// NALU data
	memcpy(&body[i], data, size);

	bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i + size, nTimeStamp);

	delete[] body;

	return bRet;
}

int CRTMPStream::getFramesSum(const char *pFrameDataDir) {
	DIR *dir = opendir(pFrameDataDir);
	struct dirent *ent = NULL;
	int sum = 0;

	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") != 0
					&& strcmp(ent->d_name, "..") != 0) {
				sum++;
			}
		}
		closedir(dir);
		return sum;
	} else {
		printf("Failed to open directory\n");
		return -1;
	}
}

int CRTMPStream::getFrameData(string fileName, unsigned char *frameBuffer,
		unsigned int frameBufferSize) {
	if (frameBuffer == 0) {
		printf("error: FrameBuffer hasn't allocate yet!\n");
		return -1;
	}

	memset(frameBuffer, 0, frameBufferSize);
	FILE *fp;
	int readFrameSize = -1;

	fp = fopen(fileName.c_str(), "rb");
	if (!fp) {
		printf("ERROR:open file %s failed!", fileName);
		return -1;
	}

	fseek(fp, 0, SEEK_SET);
	readFrameSize = fread(frameBuffer, sizeof(unsigned char), frameBufferSize,
			fp);
	if (readFrameSize < 0) {
		printf("error : Read file failed!\n");
		return -1;
	}

	if (readFrameSize == 0) {
		printf("warning : Frame data read is empty!\n");
	}

	if (readFrameSize > frameBufferSize) {
		printf("warning : File size is larger than BUFSIZE\n");
	}
	fclose(fp);
	return readFrameSize;
}

void CRTMPStream::releaseNalus(vector<NaluUnit*> nalusUnits) {
	for(int i=0;i<nalusUnits.size();i++) {
		delete nalusUnits[i];
	}
}

bool CRTMPStream::sendFirstFrame(unsigned char* data, int length) {
	vector<NaluUnit*> naluUnits = processFirstFrame(data, length);
	handleNalus(naluUnits);
	return true;
}

bool CRTMPStream::sendNormalFrame(unsigned char* data, int length) {
	vector<NaluUnit*> naluUnits = processNormalFrame(data, length);
	handleNalus(naluUnits);
	return true;
}

vector<NaluUnit*> CRTMPStream::processFirstFrame(unsigned char* data, unsigned int length) {
	int i = 0;

	bool isFoundStartCode = false;
	vector<NaluUnit*> results;

	// Record divides' start index in the frame buffer
	vector<unsigned int> divides;

	// Record the divides' length of the divide's start index,
	// such as length of 0x00 00 01 = 3,
	// length of 0x00 00 00 01 = 4
	vector<unsigned int> dividesLengths;

	while (i+3 < length) {
		// start code is 0x00 00 01
		if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x01) {
			divides.push_back(i);
			dividesLengths.push_back(3);
			i = i + 3;
			// start code is 0x00 00 00 01
		} else if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x00
				&& data[i + 3] == 0x01) {
			divides.push_back(i);
			dividesLengths.push_back(4);
			i = i + 4;
		} else {
			i++;
		}
	}
	divides.push_back(length-1);
	dividesLengths.push_back(0);

	for(int j=0;j<divides.size()-1;j++) {
		int nalIndex = divides[j] + dividesLengths[j];
		unsigned char nal = data[nalIndex];

		NaluUnit *naluUnit = new NaluUnit();
		if(j<divides.size()-2) {
			naluUnit->size = divides[j+1] - nalIndex;
		} else {
			naluUnit->size = divides[j+1] - nalIndex + 1;
		}

		naluUnit->type = nal & 0x1f;
		naluUnit->data = &data[divides[j] + dividesLengths[j]];

		results.push_back(naluUnit);
	}
	return results;
}

vector<NaluUnit*> CRTMPStream::processNormalFrame(unsigned char *data, unsigned int length) {
	vector<NaluUnit*> results;
	NaluUnit *naluUnit = new NaluUnit();

	int i = 0;
	int startLength = 0;

	if(i+1 >= length || i+2 >= length || i+3 >= length) {
		return results;
	}

	if(data[i] == 0x00 && data[i+1] == 0x00 && data[i+2] == 0x01) {
		startLength = 3;
	}

	if(data[i] == 0x00 && data[i+1] == 0x00 && data[i+2] == 0x00 && data[i+3] == 0x01) {
		startLength = 4;
	}

	naluUnit->size = length - startLength;
	naluUnit->type = data[startLength] & 0x1f;
	naluUnit->data = &data[startLength];


	results.push_back(naluUnit);
	return results;
}

void CRTMPStream::handleNalus(vector<NaluUnit*> nalus) {
	int naluNr = 0;
	RTMPMetadata metaData;

	for(int j=0;j<nalus.size();j++) {
		switch(nalus[j]->type) {
		// SPS
		case 0x07: {
			metaData.nSpsLen = nalus[j]->size;
			memcpy(metaData.Sps, nalus[j]->data, nalus[j]->size);

			int width = 0, height = 0;
			h264_decode_sps(metaData.Sps, metaData.nSpsLen, width, height);
			metaData.nWidth = width;
			metaData.nHeight = height;
			metaData.nFrameRate = frameRate;

			break;
		}

		// PPS
		case 0x08: {
			metaData.nPpsLen = nalus[j]->size;
			memcpy(metaData.Pps, nalus[j]->data, nalus[j]->size);
			break;
		}

		// SEI
		case 0x06: {
			tick += getTick();	//40是由帧数决定的,25帧情况下:tick=1s/25=0.04s=40ms
			SendH264Packet(nalus[j]->data, nalus[j]->size, false, tick);
			// msleep(frameRate);
			break;
		}

		// IDR
		case 0x05: {
			tick += getTick();
			SendH264Packet(nalus[j]->data, nalus[j]->size, true, tick);
			break;
		}

		// Not IDR
		case 0x01: {
			tick += getTick();
			SendH264Packet(nalus[j]->data, nalus[j]->size, false, tick);
			// printf("Sending %d frame...\n", naluNr++);
			break;
		}

		default: {
			printf("error: unknow frame type: %x\n", nalus[j]->type);
		}
		}

		if(j==1 && metaData.nSpsLen > 0 && metaData.nPpsLen > 0) {
			printf("Sending meta data...\n");
			// 发送MetaData
			SendMetadata(&metaData);
		}
	}
	releaseNalus(nalus);
}

unsigned int CRTMPStream::getTick() {
	long currentTime = getCurrentTime();
	long lastTimetTemp = lastTime;
	lastTime = currentTime;
	return (lastTimetTemp > 0)?(currentTime-lastTimetTemp):0;
}

bool CRTMPStream::SendH264File(const char* pFileName) {
	unsigned int frameBufferSize = 4 * 1024 * 1024;
	unsigned char* frameBuffer = new unsigned char[frameBufferSize]();
	int readFrameSize = -1;
	string fileName = pFileName;

	if ((readFrameSize = getFrameData(fileName, frameBuffer, frameBufferSize)) > 0) {
		vector<NaluUnit*> naluUnits = processFirstFrame(frameBuffer, readFrameSize);
		handleNalus(naluUnits);
	}
}

bool CRTMPStream::SendH264Frames(const char *pFrameDataDir) {
	int framesSum = getFramesSum(pFrameDataDir);
	string dirStr = pFrameDataDir;
	string fileName;
	unsigned int frameBufferSize = 2 * 1024 * 1024;
	unsigned char* frameBuffer = new unsigned char[frameBufferSize]();
	int readFrameSize = -1;

	for (int i = 0; i < framesSum; i++) {
		stringstream ss;
		ss << dirStr << i;
		ss >> fileName;
//		fileName = dirStr + i;
		// cout << "Now read frame " << fileName << endl;
		if ((readFrameSize = getFrameData(fileName, frameBuffer, frameBufferSize)) > 0) {
			if (frameBuffer[4] == 103) {
				// fetch SPS, PPS, SEI, IDR's imformation
				vector<NaluUnit*> naluUnits = processFirstFrame(frameBuffer, readFrameSize);
				handleNalus(naluUnits);
			} else {
				// send normal frame
				vector<NaluUnit*> naluUnits = processNormalFrame(frameBuffer, readFrameSize);
				handleNalus(naluUnits);
			}
		}

	}

	delete [] frameBuffer;
	return TRUE;
}
