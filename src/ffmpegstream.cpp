#include "stdafx.h"
#include "ffmpegstream.h"

static AVRational TIME_BASE = {1, 1000000};

ffmpegReader::ffmpegReader()
	:pFmtContext_(0),
	pCodecCtx_(0),
	state_(ffstateNo),
	getBufferCB_(0),
	readyBufferCB_(0),
	errorCB_(0),
	hThread_(0),
	stopPlaying_(false),
	video_stream_no_(-1)
{

}

ffmpegReader::~ffmpegReader()
{
	if(state_ != ffstateNo)
	{
		ReqCloseSource();
		WaitForStop(INFINITE);
		if(pCodecCtx_)
			avcodec_close(pCodecCtx_);
		if(pFmtContext_)
			avformat_free_context(pFmtContext_);
	}
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
	for(unsigned int i = 0; i < pFmtContext_->nb_streams; ++i) 
	{
		pStream = pFmtContext_->streams[i];
		if(pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
		{
			pCodecCtx_ = pStream->codec;
			video_stream_no_ = i;
			break;
		}
	}

	if(video_stream_no_ == -1)
		return rtsperrStreamNotFound;

	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx_->codec_id);
	if(!pCodec)
	{
		avcodec_close(pCodecCtx_);
		avformat_free_context(pFmtContext_);
		pCodecCtx_ = 0;
		pFmtContext_ = 0;
		return rtsperrCodecNotFound;
	}
	err = avcodec_open2(pCodecCtx_, pCodec, NULL);
	if(err)
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

rtspError ffmpegReader::StartStream(getBufferFunc getBufferCB, readyBufferFunc readyBufferCB, errorFunc errorCB)
{
	if(state_ != ffstateOpen)
		return rtsperrInvalidState;
	if(!getBufferCB || !readyBufferCB || !errorCB)
		return rtsperrPointer;

	getBufferCB_ = getBufferCB;
	readyBufferCB_ = readyBufferCB;
	errorCB_ = errorCB;

	stopPlaying_ = false;
	hThread_ = ::CreateThread(0, 0, PlayLoopProc, this, 0, 0);

	return rtsperrOK;
}

rtspError ffmpegReader::ReqCloseSource()
{
	stopPlaying_ = true;
	return rtsperrOK;
}

DWORD ffmpegReader::PlayLoopProc(LPVOID aParam)
{
	ffmpegReader* This = static_cast<ffmpegReader*>(aParam);
	return This->playLoop();
}

rtspError ffmpegReader::converToRGBA(AVFrame* source, TargetPicture* target)
{
	TargetBuffers buffers;
	int ret = 0;
	AVFrame*	targetFrame = av_frame_alloc();
	int numBytesY = av_image_get_buffer_size(AV_PIX_FMT_GRAY8, pCodecCtx_->width,pCodecCtx_->height, 16);
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, pCodecCtx_->width,pCodecCtx_->height, 16);
	buffers.aligment = 16;
	buffers.bgra_buffer = 0;
	buffers.y_buffer = 0;
	buffers.y_size = numBytesY;
	buffers.bgra_size = numBytes;
	rtspError err = getBufferCB_(&buffers);
	if(err != rtsperrOK)
		return rtsperrOutOfMemory;

	if(buffers.bgra_buffer)
	{
		avpicture_fill((AVPicture *) targetFrame, buffers.bgra_buffer, AV_PIX_FMT_BGRA, pCodecCtx_->width, pCodecCtx_->height);
		struct SwsContext * img_convert_ctx;
		img_convert_ctx = sws_getCachedContext(NULL,pCodecCtx_->width, pCodecCtx_->height, pCodecCtx_->pix_fmt, 
			pCodecCtx_->width, pCodecCtx_->height, AV_PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL,NULL);
		if(!img_convert_ctx)
			return rtsperrUnsupportedFormat;
		sws_scale(img_convert_ctx, ((AVPicture*)source)->data, ((AVPicture*)source)->linesize, 0, 
			pCodecCtx_->height, ((AVPicture *)targetFrame)->data, ((AVPicture *)targetFrame)->linesize);
		target->bgra_buffer = targetFrame->data[0];
		target->bgra_stride = targetFrame->linesize[0];
		target->width = source->width;
		target->height = source->height;
	}
	if(buffers.y_buffer)
	{
		avpicture_fill((AVPicture *) targetFrame, buffers.y_buffer, AV_PIX_FMT_GRAY8, pCodecCtx_->width, pCodecCtx_->height);
		struct SwsContext * img_convert_ctx;
		img_convert_ctx = sws_getCachedContext(NULL,pCodecCtx_->width, pCodecCtx_->height, pCodecCtx_->pix_fmt, 
			pCodecCtx_->width, pCodecCtx_->height, AV_PIX_FMT_GRAY8, SWS_FAST_BILINEAR, NULL, NULL,NULL);
		if(!img_convert_ctx)
			return rtsperrUnsupportedFormat;
		sws_scale(img_convert_ctx, ((AVPicture*)source)->data, ((AVPicture*)source)->linesize, 0, 
			pCodecCtx_->height, ((AVPicture *)targetFrame)->data, ((AVPicture *)targetFrame)->linesize);
		target->y_buffer = targetFrame->data[0];
		target->y_stride = targetFrame->linesize[0];
		target->width = source->width;
		target->height = source->height;
	}
	target->user_data = buffers.user_data;
	av_frame_free(&targetFrame);
	return rtsperrOK;
}

DWORD ffmpegReader::playLoop()
{
	AVPacket pkt;
	av_init_packet(&pkt);
	AVFrame*	Frame = av_frame_alloc();
	TargetPicture target_buffers;
	rtspError err;
	
	//av_frame_unref(&Frame);

	state_ = ffstateStreamming;
	while(!stopPlaying_)
	{
		int ret = av_read_frame(pFmtContext_, &pkt);
		if(ret == AVERROR_EOF)
		{
			errorCB_(rtsperrEOF);
			state_= ffstateError;
			return rtsperrEOF;
		}
		else if(ret != 0)
		{
			errorCB_(rtsperrReadError);
			state_= ffstateError;
			return rtsperrReadError;
		}
		else
		{
			if(pkt.stream_index != video_stream_no_)
				continue;
			int gotAFrame;
			ret = avcodec_decode_video2(pCodecCtx_, Frame, &gotAFrame, &pkt);
			if(ret < 0)
			{
				errorCB_(rtsperrDecodeError);
				state_= ffstateError;
				return rtsperrDecodeError;
			}
			if(gotAFrame)
			{
				err = converToRGBA(Frame, &target_buffers);
				if(err != rtsperrOK)
				{
					errorCB_(err);
					state_= ffstateError;
					return err;
				}
				target_buffers.us_pts = av_rescale_q(pkt.pts, pFmtContext_->streams[pkt.stream_index]->time_base, TIME_BASE);
				readyBufferCB_(&target_buffers);
			}
		}
	}
	state_ = ffstateStoped;
	return 0;
}

rtspError ffmpegReader::WaitForStop(DWORD timeout)
{
	if(!hThread_)
		return rtsperrOK;
	DWORD ret = WaitForSingleObject(hThread_, timeout);
	if(WAIT_OBJECT_0 == ret)
	{
		CloseHandle(hThread_);
		hThread_ = 0;
		return rtsperrOK;
	}
	else if(WAIT_TIMEOUT == ret)
		return rtsperrTimeOut;
	else
		return rtsperrFailed;
}