#pragma once

#define DllImport   __declspec( dllimport )
#define DllExport   __declspec( dllexport )

#ifdef RTSPSRC_EXPORTS
#define DLL_Linkage DllExport
#else
#define DLL_Linkage DllImport
#endif

typedef void* rtspSession;

typedef enum
{
	rtsperrOK = 0,
	rtsperrEOF = 1,
	rtsperrFailed = -1,
	rtsperrPointer = -2,
	rtsperrNonZeroSession = -3,
	rtsperrStreamNotFound = -4,
	rtsperrFaileStreamOpen = -5,
	rtsperrCodecNotFound = -6,
	rtsperrInvalidState = -7,
	rtsperrReadError = -8,
	rtsperrDecodeError = -9,
} rtspError;

typedef unsigned char* (*getBufferFunc)(size_t bufferSize);
typedef void (*readyBufferFunc)(int width, int height, int stride, unsigned char* buffer);
typedef void (*errorFunc)(rtspError error);

DLL_Linkage void rtspInitSource();
DLL_Linkage rtspError rtspOpenSource(const char* srcURL, rtspSession* session);
DLL_Linkage rtspError rtspStartStream(rtspSession session, getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB);
DLL_Linkage rtspError rtspCloseSource(rtspSession session);