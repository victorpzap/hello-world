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
	rtsperrTimeOut = -10,
	rtsperrOutOfMemory = -11,
	rtsperrUnsupportedFormat = -12,
} rtspError;

typedef struct
{
	int width;
	int height;
	unsigned char* y_buffer;
	int y_stride;
	unsigned char* bgra_buffer;
	int bgra_stride;
	__int64	ms_pts;
	void* user_data;
} TargetPicture;

typedef struct
{
	int y_size;
	int bgra_size;
	int aligment;
	unsigned char* y_buffer;
	unsigned char* bgra_buffer;
	void* user_data;
} TargetBuffers;

typedef rtspError (*getBufferFunc)(TargetBuffers* buffers);
typedef void (*readyBufferFunc)(TargetPicture* picture);
typedef void (*errorFunc)(rtspError error);

DLL_Linkage void rtspInitSource();
DLL_Linkage rtspError rtspOpenSource(const char* srcURL, rtspSession* session);
DLL_Linkage rtspError rtspStartStream(rtspSession session, getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB);
DLL_Linkage rtspError rtspCloseStream(rtspSession session);
DLL_Linkage rtspError rtspWaitForStop(rtspSession session, unsigned int timeout);
DLL_Linkage rtspError rtspCloseSource(rtspSession session);
