#pragma once

#include "..\inc\rtspsrc.h"
#include <Windows.h>

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavformat/avio.h>
}

struct sourceParams
{
	int width;
	int height;
};

class ffmpegReader
{
public:
	enum ffstate 
	{
		ffstateNo, 
		ffstateOpen, 
		ffstateStreamming, 
		ffstateStoped, 
		ffstateError, 
	};  
	ffmpegReader();
	~ffmpegReader();
	rtspError openStream(const char* srcURL);
	rtspError getParams(sourceParams* params);
	rtspError StartStream(getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB);
	rtspError ReqCloseSource();
	rtspError WaitForStop(DWORD timeout);
	static int InterruptCBFunc(void/** ptr*/);
protected:
	static DWORD WINAPI PlayLoopProc(LPVOID aParam);
	DWORD playLoop();
	rtspError converToRGBA(AVFrame* source, TargetPicture* target);


private:
	AVFormatContext* pFmtContext_;
	AVCodecContext* pCodecCtx_;
	int		video_stream_no_;
	ffstate state_;
	getBufferFunc getBufferCB_;
	readyBufferFunc readyBufferCB_;
	errorFunc errorCB_;
	HANDLE	hThread_;
	volatile bool	stopPlaying_;
};