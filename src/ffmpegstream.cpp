#include "stdafx.h"
#include "ffmpegstream.h"

ffmpegReader::ffmpegReader()
	:pFmtContext_(0),
	pCodecCtx_(0),
	state_(ffstateNo)
{

}

ffmpegReader::~ffmpegReader()
{
}

int ffmpegReader::openStream(const char* srcURL)
{
	if(state_ != ffstateNo)
		return rtsperrInvalidState;

	AVDictionary *options = 0;
	int err = avformat_open_input(&pFmtContext_, srcURL, 0, &options);
	if(err)
		return rtsperrFaileStreamOpen;

	AVStream *	pStream;
	size_t	streamNo = -1;
	for(unsigned int i = 0; i < pFmtContext_->nb_streams; ++i) 
	{
		pStream = pFmtContext_->streams[i];
		if(pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
		{
			streamNo = i;
			pCodecCtx_ = pStream->codec;
			break;
		}
	}

	if(streamNo == -1)
		return rtsperrStreamNotFound;

	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx_->codec_id);
	if(pCodec == 0)
	{
		avcodec_close(pCodecCtx_);
		avformat_free_context(pFmtContext_);
		pCodecCtx_ = 0;
		pFmtContext_ = 0;
		return rtsperrCodecNotFound;
	}
	state_ = ffstateOpen;
	return rtsperrOK;
}

int getParams(sourceParams* params)
{
	return rtsperrOK;
}

int ffmpegReader::InterruptCBFunc(void/** ptr*/)
{
	return 0;
}