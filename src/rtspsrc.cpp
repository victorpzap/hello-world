// rtspsrc.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <iostream>
#include <memory>
#include <assert.h>

#include "../inc/rtspsrc.h"
#include "ffmpegstream.h"


DLL_Linkage void rtspInitSource()
{
	av_register_all();
}

DLL_Linkage rtspError rtspOpenSource(const char* srcURL, rtspSession* session)
{
	if(!session)
		return rtsperrPointer;

	if(*session)
		return rtsperrNonZeroSession;

	std::auto_ptr<ffmpegReader> reader(new ffmpegReader);
	int err = reader->openStream(srcURL);
	if(err)
		return rtsperrFailed;

	*session = static_cast<rtspSession>(reader.release());
	return rtsperrOK;
}

DLL_Linkage rtspError rtspStartStream(rtspSession session, getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB)
{
	if(!session)
		return rtsperrPointer;
	ffmpegReader* reader = static_cast<ffmpegReader*>(session);
	return reader->StartStream(getBufferCB, readyBufferCB, errorCB);
}

DLL_Linkage rtspError rtspCloseStream(rtspSession session)
{
	if(!session)
		return rtsperrPointer;
	ffmpegReader* reader = static_cast<ffmpegReader*>(session);
	reader->ReqCloseSource();
	return rtsperrOK;
}

DLL_Linkage rtspError rtspWaitForStop(rtspSession session, unsigned int timeout)
{
	if(!session)
		return rtsperrPointer;
	ffmpegReader* reader = static_cast<ffmpegReader*>(session);
	return reader->WaitForStop(timeout);
}

DLL_Linkage rtspError rtspCloseSource(rtspSession session)
{
	if(!session)
		return rtsperrPointer;
	ffmpegReader* reader = static_cast<ffmpegReader*>(session);
	delete reader;
	return rtsperrOK;
}