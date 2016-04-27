// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../inc/rtspsrc.h"
#include <malloc.h>
#include <string>

unsigned char* YBuffer = 0;
size_t YBufferSize = 0;
unsigned char* BgraBuffer = 0;
size_t BgraBufferSize = 0;

FILE* f = 0;

rtspError getBuffer(TargetBuffers* buffers)
{
	if(buffers->y_size > YBufferSize || !YBuffer)
	{
		if(YBuffer)
			_aligned_free(YBuffer);
		YBuffer = (unsigned char*)_aligned_malloc(buffers->y_size, buffers->aligment);
		YBufferSize = buffers->y_size;
	}
	buffers->y_buffer = YBuffer;
	if(!buffers->y_buffer)
		return rtsperrOutOfMemory;

	if(buffers->bgra_size > BgraBufferSize || !BgraBuffer)
	{
		if(BgraBuffer)
			_aligned_free(BgraBuffer);
		BgraBuffer = (unsigned char*)_aligned_malloc(buffers->bgra_size, buffers->aligment);
		BgraBufferSize = buffers->bgra_size;
	}
	buffers->bgra_buffer = BgraBuffer;

	return BgraBuffer ? rtsperrOK : rtsperrOutOfMemory;
}

void freeBuffers()
{
	if(YBuffer)
		_aligned_free(YBuffer);
	if(BgraBuffer)
		_aligned_free(BgraBuffer);
}

void readyBuffer(TargetPicture* picture)
{
	if(f)
	{
		fwrite(picture->bgra_buffer, sizeof(char), picture->bgra_stride * picture->height, f);
	}
	Sleep(10);
	printf("* %I64d\n", picture->us_pts);
}

void processError(rtspError err)
{
	printf("Rised error %d\n", err);
};

/*
* Test for rtspsrc
* 
*/
int _tmain(int argc, _TCHAR* argv[])
{
//	const char* src = "C:/333/5555/axis&_15_06_24_21_15_36_459.avi";
	std::string src = "C:/333/36.avi";
	std::string targ = "C:/333/out.yuv";

	if(argc >= 2)
		src = argv[1];
	if(argc >= 3)
		targ = argv[2];

	rtspSession session = 0;
	rtspInitSource();

	int err = rtspOpenSource(src.c_str(), &session);
	if(err != rtsperrOK)
	{
		printf("rtspOpenSource rised error %d\n", err);
		return 1;
	}
	if(targ.length())
	{
		f = fopen(targ.c_str(), "wb");
	}

	err = rtspStartStream(session, getBuffer, readyBuffer, processError);
	int a;
	while((a = getchar()) != 's');
	rtspCloseStream(session);
	rtspWaitForStop(session, INFINITE);
	rtspCloseSource(session);
	freeBuffers();
	if(f)
		fclose(f);
	return 0;
}

