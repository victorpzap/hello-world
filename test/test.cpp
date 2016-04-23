// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../inc/rtspsrc.h"
#include <malloc.h>

unsigned char* pBuffer = 0;
size_t bufferSize = 0;

unsigned char* getBuffer(size_t size)
{
	if(size > bufferSize || !pBuffer)
	{
		pBuffer = (unsigned char*)_aligned_malloc(size, 16);
		bufferSize = size;
	}
	return pBuffer;
}

void freeBuffer()
{
	if(pBuffer)
		_aligned_free(pBuffer);
}

int _tmain(int argc, _TCHAR* argv[])
{
	const char* src = "C:/333/5555/axis&_15_06_24_21_15_36_459.avi";
	rtspSession session = 0;
	rtspInitSource();

	int err = rtspOpenSource(src, getBuffer, &session);

	freeBuffer();
	return 0;
}

