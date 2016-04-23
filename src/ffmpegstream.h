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
	enum ffstate {ffstateNo, ffstateOpen,};  
	ffmpegReader();
	~ffmpegReader();
	rtspError openStream(const char* srcURL);
	rtspError getParams(sourceParams* params);
	rtspError rtspStartStream(getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB);
	rtspError rtspCloseSource();
	static int InterruptCBFunc(void/** ptr*/);
protected:
	static DWORD WINAPI PlayLoopProc(LPVOID aParam);
	DWORD playLoop();
	rtspError converToRGBA(AVFrame* source, unsigned char* target);

private:
	AVFormatContext* pFmtContext_;
	AVCodecContext* pCodecCtx_;
	ffstate state_;
	getBufferFunc getBufferCB_;
	readyBufferFunc readyBufferCB_;
	errorFunc errorCB_;
	HANDLE	hThread_;
	bool	stopPlaying;
};