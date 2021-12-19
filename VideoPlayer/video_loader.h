#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#ifndef MAX_CODECS 
#define MAX_CODECS 2
#endif

struct Codec
{
	AVCodecParameters *codec_params;
	AVCodec const *codec;
	AVCodecContext *codec_context;
	int32_t stream_index;
};

class video_loader
{
	enum : uint32_t
	{
		CODEC_VIDEO = 0,
		CODEC_AUDIO = 1
	};

	AVFormatContext *format_context_;
	Codec codec_[MAX_CODECS];
	int32_t width, height;
	int32_t found_codec_count;

	void make_error(int error_num);
	AVPixelFormat correct_for_deprecated_pixel_format(AVPixelFormat pix_fmt);
public:
	video_loader();

	void open_file(const char *file_path, int *width, int *height);
	void get_frame(uint8_t **out_frame_data);

	void close();
};
