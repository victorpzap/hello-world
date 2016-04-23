#pragma once

#include "..\inc\rtspsrc.h"
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
	int openStream(const char* srcURL);
	int getParams(sourceParams* params);
	static int InterruptCBFunc(void/** ptr*/);

private:
	AVFormatContext* pFmtContext_;
	AVCodecContext* pCodecCtx_;
	ffstate state_;
};