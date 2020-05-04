---
layout: post
title:  "FFmpeg을 한번 써보자 (3)"
date:   2020-04-30
comments: true
excerpt: "ffmpeg tutorial03"
categories: ffmpeg
tag:
- c
- multimedia 
- ffmpeg 
---

# demuxing 했으니 remuxing쯤은....??

```c++
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

typedef struct _FileContext
{
  AVFormatContext* fmt_ctx;
  int v_index;
  int a_index;
} FileContext;

static FileContext inputFile, outputFile;

static int open_input(const char* fileName)
{
  unsigned int index;

  inputFile.fmt_ctx = NULL;
  inputFile.a_index = inputFile.v_index = -1;

  if(avformat_open_input(&inputFile.fmt_ctx, fileName, NULL, NULL) < 0)
  {
    printf("Could not open input file %s\n", fileName);
    return -1;
  }

  if(avformat_find_stream_info(inputFile.fmt_ctx, NULL) < 0)
  {
    printf("Failed to retrieve input stream information\n");
    return -2;
  }

  for(index = 0; index < inputFile.fmt_ctx->nb_streams; index++)
  {
    AVCodecContext* codec_ctx = inputFile.fmt_ctx->streams[index]->codec;
    if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && inputFile.v_index < 0)
    {
      inputFile.v_index = index;
    }
    else if(codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && inputFile.a_index < 0)
    {
      inputFile.a_index = index;
    }
  } // for

  if(inputFile.v_index < 0 && inputFile.a_index < 0)
  {
    printf("Failed to retrieve input stream information\n");
    return -3;
  }

  return 0;
}

static int create_output(const char* fileName)
{
  unsigned int index;
  int out_index;

  outputFile.fmt_ctx = NULL;
  outputFile.a_index = outputFile.v_index = -1;

  if(avformat_alloc_output_context2(&outputFile.fmt_ctx, NULL, NULL, fileName) < 0)
  {
    printf("Could not create output context\n");
    return -1;
  }

  // stream index starts from 0.
  out_index = 0;
  // this copy video/audio streams from input video.
  for(index = 0; index < inputFile.fmt_ctx->nb_streams; index++)
  {
    // Make sure we only copy streams which is checked before.
    if(index != inputFile.v_index && index != inputFile.a_index)
    {
      continue;
    }

    AVStream* in_stream = inputFile.fmt_ctx->streams[index];
    AVCodecContext* in_codec_ctx = in_stream->codec;

    AVStream* out_stream = avformat_new_stream(outputFile.fmt_ctx, in_codec_ctx->codec);
    if(out_stream == NULL)
    {
      printf("Failed to allocate output stream\n");
      return -2;
    }

    AVCodecContext* outCodecContext = out_stream->codec;
    if(avcodec_copy_context(outCodecContext, in_codec_ctx) < 0)
    {
      printf("Error occurred while copying context\n");
      return -3;
    }

    // Use AVStream instead of AVCodecContext(Deprecated).
    out_stream->time_base = in_stream->time_base;
    // Remove codec tag info for compatibility with ffmpeg.
    outCodecContext->codec_tag = 0;
    if(outputFile.fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
      outCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    if(index == inputFile.v_index)
    {
      outputFile.v_index = out_index++;
    }
    else
    {
      outputFile.a_index = out_index++;
    }
  } // for

  if(!(outputFile.fmt_ctx->oformat->flags & AVFMT_NOFILE))
  {
    // This actually open the file
    if(avio_open(&outputFile.fmt_ctx->pb, fileName, AVIO_FLAG_WRITE) < 0)
    {
      printf("Failed to create output file %s\n", fileName);
      return -4;
    }
  }

  // write the header for output video container.
  if(avformat_write_header(outputFile.fmt_ctx, NULL) < 0)
  {
    printf("Failed writing header into output file\n");
    return -5;  
  }

  return 0;
}

static void release()
{
  if(inputFile.fmt_ctx != NULL)
  {
    avformat_close_input(&inputFile.fmt_ctx);
  }

  if(outputFile.fmt_ctx != NULL)
  {
    if(!(outputFile.fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
      avio_closep(&outputFile.fmt_ctx->pb);
    }
    avformat_free_context(outputFile.fmt_ctx);
  }
}

int main(int argc, char* argv[])
{
  int ret;

  av_register_all();
  av_log_set_level(AV_LOG_DEBUG);

  if(argc < 3)
  {
    printf("usage : %s <input> <output>\n", argv[0]);
    return 0;
  }

  if(open_input(argv[1]) < 0)
  {
    goto main_end;
  }

  if(create_output(argv[2]) < 0)
  {
    goto main_end;
  }

  // dump output container, which i just make from above.
  av_dump_format(outputFile.fmt_ctx, 0, outputFile.fmt_ctx->filename, 1);

  AVPacket pkt;
  int out_stream_index;

  while(1)
  {
    ret = av_read_frame(inputFile.fmt_ctx, &pkt);
    if(ret == AVERROR_EOF)
    {
      printf("End of frame\n");
      break;
    }

    if(pkt.stream_index != inputFile.v_index && 
      pkt.stream_index != inputFile.a_index)
    {
      av_free_packet(&pkt);
      continue;
    }

    AVStream* in_stream = inputFile.fmt_ctx->streams[pkt.stream_index];
    out_stream_index = (pkt.stream_index == inputFile.v_index) ? 
            outputFile.v_index : outputFile.a_index;
    AVStream* out_stream = outputFile.fmt_ctx->streams[out_stream_index];

    av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);

    pkt.stream_index = out_stream_index;

    if(av_interleaved_write_frame(outputFile.fmt_ctx, &pkt) < 0)
    {
      printf("Error occurred when writing packet into file\n");
      break;
    }   
  } // while

  // Writes remain informations, which it is called trailer.
  av_write_trailer(outputFile.fmt_ctx);

main_end:
  release();

  return 0;
}
```
일단 main부터 보자! 잘보면 tutorial02와 상당히 비슷하다. 우선 open_input까지는 동일하고 create_output과 while문에서 av_read_frame이후에 뭔가의 동작을 추가적으로 하는 것으로 보인다. 먼저 create_output을 살펴보자

```c++
static int create_output(const char* fileName)
{
  unsigned int index;
  int out_index;

  outputFile.fmt_ctx = NULL;
  outputFile.a_index = outputFile.v_index = -1;

  if(avformat_alloc_output_context2(&outputFile.fmt_ctx, NULL, NULL, fileName) < 0)
  {
    printf("Could not create output context\n");
    return -1;
  }

  // stream index starts from 0.
  out_index = 0;
  // this copy video/audio streams from input video.
  for(index = 0; index < inputFile.fmt_ctx->nb_streams; index++)
  {
    // Make sure we only copy streams which is checked before.
    if(index != inputFile.v_index && index != inputFile.a_index)
    {
      continue;
    }

    AVStream* in_stream = inputFile.fmt_ctx->streams[index];
    AVCodecContext* in_codec_ctx = in_stream->codec;

    AVStream* out_stream = avformat_new_stream(outputFile.fmt_ctx, in_codec_ctx->codec);
    if(out_stream == NULL)
    {
      printf("Failed to allocate output stream\n");
      return -2;
    }

    AVCodecContext* outCodecContext = out_stream->codec;
    if(avcodec_copy_context(outCodecContext, in_codec_ctx) < 0)
    {
      printf("Error occurred while copying context\n");
      return -3;
    }

    // Use AVStream instead of AVCodecContext(Deprecated).
    out_stream->time_base = in_stream->time_base;
    // Remove codec tag info for compatibility with ffmpeg.
    outCodecContext->codec_tag = 0;
    if(outputFile.fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
      outCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    if(index == inputFile.v_index)
    {
      outputFile.v_index = out_index++;
    }
    else
    {
      outputFile.a_index = out_index++;
    }
  } // for

  if(!(outputFile.fmt_ctx->oformat->flags & AVFMT_NOFILE))
  {
    // This actually open the file
    if(avio_open(&outputFile.fmt_ctx->pb, fileName, AVIO_FLAG_WRITE) < 0)
    {
      printf("Failed to create output file %s\n", fileName);
      return -4;
    }
  }

  // write the header for output video container.
  if(avformat_write_header(outputFile.fmt_ctx, NULL) < 0)
  {
    printf("Failed writing header into output file\n");
    return -5;  
  }

  return 0;
}
```
우선 `avformat_alloc_output_context2`에 대해서 알아보자
```
int avformat_alloc_output_context2 (AVFormatContext **ctx, AVOutputFormat *oformat, const char *format_name, const char *filename) : Allocate an AVFormatContext for an output format.

avformat_free_context() can be used to free the context and everything allocated by the framework within it.
Parameters
*ctx	is set to the created format context, or to NULL in case of failure
oformat	format to use for allocating the context, if NULL format_name and filename are used instead
format_name	the name of output format to use for allocating the context, if NULL filename is used instead
filename	the name of the filename to use for allocating the context, may be NULL
```
설명에 따르면 ctx에 AVFormatContext를 allocate해주는데, input으로 들어온 값들에 맞추어 allocate을 수행해준다고 한다. 
그 이후 쭉 내려가다가

AVStream* out_stream = avformat_new_stream(outputFile.fmt_ctx, in_codec_ctx->codec);
여기에서 avformat_new_stream을 통해서 해당 코덱에 맞는 stream을 생성한다.
```
AVStream* avformat_new_stream	(AVFormatContext* s, const AVCodec* c) :
Add a new stream to a media file.
When demuxing, it is called by the demuxer in read_header(). If the flag AVFMTCTX_NOHEADER is set in s.ctx_flags, then it may also be called in read_packet().
When muxing, should be called by the user before avformat_write_header().
User is required to call avcodec_close() and avformat_free_context() to clean up the allocation by avformat_new_stream().
```
대충 설명을 보면 파일을 읽어들일때도 stream이 필요할거 같은데, 그 경우에는 read_header에서 stream을 만들어주기 때문에 필요없다고 한다. 이번경우에는 user가 파일을 만들어 주는 것이기 때문에 stream을 new할 필요가 있다.

```
int avcodec_copy_context (AVCodecContext* dest, const AVCodecContext* src) : Copy the settings of the source AVCodecContext into the destination AVCodecContext.

The resulting destination codec context will be unopened, i.e. you are required to call avcodec_open2() before you can use this AVCodecContext to decode/encode video/audio data.
```
이 함수는 굳이 왜쓰는지 모르겠다

```
int 	avio_open (AVIOContext **s, const char *url, int flags) : Create and initialize a AVIOContext for accessing the resource indicated by url.
```
이제 위 함수를 통해서 실제로 파일을 열고


```
av_warn_unused_result int avformat_write_header	(	AVFormatContext * 	s,
AVDictionary ** 	options 
) : Allocate the stream private data and write the stream header to an output media file.
```
avformat_write_header를 통해서 파일에 header값을 써준다. 이제 파일에 실제 stream data를 쓸 준비가 되었다!!

이제 다시 main으로 돌아와보자

```
void av_dump_format (AVFormatContext *ic, int index, const char *url, int is_output)
Print detailed information about the input or output format, such as duration, bitrate, streams, container, programs, metadata, side data, codec and time base. 
Parameters
  ic : the context to analyze
  index : index of the stream to dump information about
  url : the URL to print, such as source or destination file
  is_output : Select whether the specified context is an input(0) or output(1) 
```
이름 그대로 input output format에 대한 정보를 출력해주는 함수로 큰 역할은 없다
이후 while문 내에서
av_read_frame을 통해 frame을 읽어 pkt에 담는다
그후 in_stream과 out_stream을 뽑아서 각 stream에 맞게 time_base를  변환해주고 여기 에 맞춰서
av_interleaved_write_frame을 통해서 pkt을 outputFile에 적어준다.

pkt를 다 쓰고 난 뒤에는 av_write_trailer를 통해 정리를 해준다.
```
int av_interleaved_write_frame (AVFormatContext *s, AVPacket *pkt)
Write a packet to an output media file ensuring correct interleaving.

This function will buffer the packets internally as needed to make sure the packets in the output file are properly interleaved in the order of increasing dts. Callers doing their own interleaving should call av_write_frame() instead of this function.

Using this function instead of av_write_frame() can give muxers advance knowledge of future packets, improving e.g. the behaviour of the mp4 muxer for VFR content in fragmenting mode.
```
```
int av_write_trailer(AVFormatContext* s)
Write the stream trailer to an output media file and free the file private data.
May only be called after a successful call to avformat_write_header.
```
