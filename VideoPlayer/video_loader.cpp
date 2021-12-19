#pragma warning disable 26812

#include "video_loader.h"
#include <iostream>

video_loader::video_loader()
{
	format_context_ = avformat_alloc_context();
	for (int i = 0; i < MAX_CODECS; i++)
	{
		codec_[i].codec = nullptr;
		codec_[i].codec_context = nullptr;
		codec_[i].codec_params = nullptr;
		codec_[i].stream_index = -1;
	}
	found_codec_count = 0;
	width = 0;
	height = 0;
}

void video_loader::open_file(const char *file_path, int *width, int *height)
{
	if(avformat_open_input(&format_context_, file_path, nullptr, nullptr) != 0)
	{
		std::cout << "Couldn't open file with the current format context";
		return;
	}

	for (uint32_t i = 0; i < format_context_->nb_streams; i++)
	{
		const auto stream = format_context_->streams[i];
		auto codec_params = stream->codecpar;
		auto codec = avcodec_find_decoder(codec_params->codec_id);

		if (!codec) continue;

		if (codec_params->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			codec_[CODEC_VIDEO].codec_params = codec_params;
			codec_[CODEC_VIDEO].codec = codec;
			codec_[CODEC_VIDEO].stream_index = i;
			found_codec_count++;
		}
		else if (codec_params->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			codec_[CODEC_AUDIO].codec_params = codec_params;
			codec_[CODEC_AUDIO].codec = codec;
			codec_[CODEC_AUDIO].stream_index = i;
			found_codec_count++;
		}
	}

	if (codec_[CODEC_VIDEO].stream_index == -1)
	{
		std::cout << "Couldn't find valid video stream";
		return;
	}
	if (codec_[CODEC_AUDIO].stream_index == -1)
	{
		std::cout << "Couldn't find valid video stream";
		return;
	}

	if (codec_[CODEC_VIDEO].codec != nullptr)
	{
		codec_[CODEC_VIDEO].codec_context = avcodec_alloc_context3(codec_[CODEC_VIDEO].codec);
		avcodec_parameters_to_context(codec_[CODEC_VIDEO].codec_context, codec_[CODEC_VIDEO].codec_params);
		avcodec_open2(codec_[CODEC_VIDEO].codec_context, codec_[CODEC_VIDEO].codec, nullptr);
	}
	if (codec_[CODEC_AUDIO].codec != nullptr)
	{
		codec_[CODEC_AUDIO].codec_context = avcodec_alloc_context3(codec_[CODEC_AUDIO].codec);
		avcodec_parameters_to_context(codec_[CODEC_AUDIO].codec_context, codec_[CODEC_AUDIO].codec_params);
		avcodec_open2(codec_[CODEC_AUDIO].codec_context, codec_[CODEC_AUDIO].codec, nullptr);
	}

	this->width = codec_[CODEC_VIDEO].codec_params->width;
	this->height = codec_[CODEC_VIDEO].codec_params->height;

	*width = this->width;
	*height = this->height;
}

void video_loader::get_frame(uint8_t** out_frame_data)
{
	AVFrame *frame = av_frame_alloc();
	AVPacket *packet = av_packet_alloc();

	auto video_codec_context = codec_[CODEC_VIDEO].codec_context;
	auto audio_codec_context = codec_[CODEC_AUDIO].codec_context;
	int const video_stream_index = codec_[CODEC_VIDEO].stream_index;
	int const audio_stream_index = codec_[CODEC_AUDIO].stream_index;
	
	AVCodecContext *current_codec_context = nullptr;

	while(av_read_frame(format_context_, packet) >= 0)
	{

		if (packet->stream_index == video_stream_index) current_codec_context = video_codec_context;
		//else if (packet->stream_index == audio_stream_index) audio_codec_context = audio_codec_context;

		if (
			packet->stream_index != video_stream_index 
			//||
			//packet->stream_index != audio_stream_index
			)
		{
			av_packet_unref(packet);
			continue;
		}

		int response = avcodec_send_packet(current_codec_context, packet);
		if(response < 0)
		{
			std::cout << "Failed to decode packet. ";
			make_error(response);
			return;
		}
		response = avcodec_receive_frame(current_codec_context, frame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
		{
			continue;
		}
		if (response < 0)
		{
			std::cout << "Failed to decode packet. ";
			make_error(response);
			return;
		}
		current_codec_context = nullptr;
		av_packet_unref(packet);
		break;
	}

	uint8_t *data = new uint8_t[frame->width * frame->height * 4];

	auto corrected_pix_format = correct_for_deprecated_pixel_format(video_codec_context->pix_fmt);

	auto *sws_context = sws_getContext(frame->width, frame->height, corrected_pix_format,
									   frame->width, frame->height, AV_PIX_FMT_RGBA,
									   SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

	if (!sws_context)
	{
		std::cout << "Couldn't initialize sws context.";
		return;
	}

	uint8_t *dest[4] = { data, nullptr, nullptr, nullptr};
	int dest_linesize[4] = {width * 4, 0, 0, 0};

	sws_scale(sws_context, frame->data, frame->linesize,
			  0, height, dest, dest_linesize);

	*out_frame_data = data;

	sws_freeContext(sws_context);
	av_packet_unref(packet);
	av_frame_unref(frame);
	av_frame_free(&frame);
	av_packet_free(&packet);
}

void video_loader::close()
{
	avformat_close_input(&format_context_);
	avformat_free_context(format_context_);
	for (int i = 0; i < MAX_CODECS && i < found_codec_count; i++)
	{
		avcodec_close(codec_[i].codec_context);
		avcodec_free_context(&codec_[i].codec_context);
		avcodec_parameters_free(&codec_[i].codec_params);
	}
}

void video_loader::make_error(int error_num)
{
	const int buffer_size = 256;
	char buffer[buffer_size];
	std::cout << av_make_error_string(buffer, buffer_size, error_num);
}

AVPixelFormat video_loader::correct_for_deprecated_pixel_format(AVPixelFormat pix_fmt)
{
// Fix swscaler deprecated pixel format warning
// (YUVJ has been deprecated, change pixel format to regular YUV)
	switch (pix_fmt)
	{
		case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
		case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
		case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
		case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
		default:                  return pix_fmt;
	}
}
