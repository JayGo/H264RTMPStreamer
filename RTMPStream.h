/******************************************************************** 
filename:   RTMPStream.h
created:    2013-04-3
author:     firehood 
purpose:    发送H264视频到RTMP Server，使用libRtmp库
*********************************************************************/ 
#pragma once
#include "librtmp/rtmp.h"
#include "librtmp/rtmp_sys.h"
#include "librtmp/amf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
using namespace std;

#define FILEBUFSIZE (1024 * 1024 * 100)       //  10M

// NALU单元
typedef struct _NaluUnit
{
	int type;
	int size;
	unsigned char *data;
}NaluUnit;

typedef struct _RTMPMetadata
{
	// video, must be h264 type
	unsigned int	nWidth;
	unsigned int	nHeight;
	unsigned int	nFrameRate;		// fps
	unsigned int	nVideoDataRate;	// bps
	unsigned int	nSpsLen;
	unsigned char	Sps[1024];
	unsigned int	nPpsLen;
	unsigned char	Pps[1024];

	// audio, must be aac type
	bool	        bHasAudio;
	unsigned int	nAudioSampleRate;
	unsigned int	nAudioSampleSize;
	unsigned int	nAudioChannels;
	char		    pAudioSpecCfg;
	unsigned int	nAudioSpecCfgLen;

} RTMPMetadata,*LPRTMPMetadata;


class CRTMPStream
{
public:
	CRTMPStream(void);
	~CRTMPStream(void);
public:
	// 连接到RTMP Server
	bool Connect(const char* url);
    // 断开连接
	void Close();
    // 发送MetaData
	bool SendMetadata(LPRTMPMetadata lpMetaData);
    // 发送H264数据帧
	bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp);
	// 发送H264文件
	bool SendH264File(const char *pFileName);
	// send H264 data frames
	bool SendH264Frames(const char *pFrameDataDir);

private:
	// 送缓存中读取一个NALU包
	bool ReadOneNaluFromBuf(NaluUnit &nalu);
	// 发送数据
	int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp);
	// get H264 frames number
	int getFramesSum(const char *pFrameDataDir);
	// process first frame, get sps, psp, sei, idr
	vector<NaluUnit*> processFirstFrame(unsigned char *data, const unsigned int length);
	// get frame data from file
	int getFrameData(string fileName, unsigned char *frameBuffer, unsigned int frameBufferSize);
	// process frames excluding first frame
	NaluUnit processNormalFrame(unsigned char *data, const unsigned int length);

private:
	RTMP* m_pRtmp;
	unsigned char* m_pFileBuf;
	unsigned int  m_nFileBufSize;
	unsigned int  m_nCurPos;
};
