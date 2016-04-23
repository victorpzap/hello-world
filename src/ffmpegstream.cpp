#include "stdafx.h"
#include "ffmpegstream.h"

ffmpegReader::ffmpegReader()
	:pFmtContext_(0),
	pCodecCtx_(0),
	state_(ffstateNo),
	getBufferCB_(0),
	readyBufferCB_(0),
	errorCB_(0),
	hThread_(0),
	stopPlaying(false)
{

}

ffmpegReader::~ffmpegReader()
{
}

rtspError ffmpegReader::openStream(const char* srcURL)
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

rtspError ffmpegReader::getParams(sourceParams* params)
{
	return rtsperrOK;
}

int ffmpegReader::InterruptCBFunc(void/** ptr*/)
{
	return 0;
}

rtspError ffmpegReader::rtspStartStream(getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB)
{
	if(state_ != ffstateOpen)
		return rtsperrInvalidState;
	if(!getBufferCB || !readyBufferCB || !errorCB)
		return rtsperrPointer;

	getBufferCB_ = getBufferCB;
	readyBufferCB_ = readyBufferCB;
	errorCB_ = errorCB;

	hThread_ = ::CreateThread(0, 0, PlayLoopProc, this, 0, 0);

	return rtsperrOK;
}

rtspError ffmpegReader::rtspCloseSource()
{
	return rtsperrOK;
}

DWORD ffmpegReader::PlayLoopProc(LPVOID aParam)
{
	ffmpegReader* This = static_cast<ffmpegReader*>(aParam);
	return This->playLoop();
}

rtspError ffmpegReader::converToRGBA(AVFrame* source, unsigned char* target)
{
	return rtsperrOK;
}

DWORD ffmpegReader::playLoop()
{
	AVPacket pkt;
	av_init_packet(&pkt);
	AVFrame	Frame;
	av_frame_unref(&Frame);

		while(!stopPlaying)
	{
		int ret = av_read_frame(pFmtContext_, &pkt);
		if(ret == AVERROR_EOF)
		{
			errorCB_(rtsperrEOF);
			return rtsperrEOF;
		}
		else if(ret != 0)
		{
			errorCB_(rtsperrReadError);
			return rtsperrReadError;
		}
		else
		{
			int gotAFrame;
			ret = avcodec_decode_video2(pCodecCtx_, &Frame, &gotAFrame, &pkt);
			if(ret < 0)
			{
				errorCB_(rtsperrDecodeError);
				return rtsperrDecodeError;
			}
			if(gotAFrame)
			{
				converToRGBA(&Frame, 0);
			}
		}

	}
	return 0;
}