---
layout: post
title:  "FFmpeg을 한번 써보자 (4)"
date:   2020-05-05
comments: true
excerpt: "ffmpeg tutorial04"
categories: ffmpeg
tag:
- c
- multimedia 
- ffmpeg 
---

#이제 decoding을 해보자!
```c++
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/common.h>
#include <libavutil/avutil.h>
#include <stdio.h>

typedef struct _FileContext
{
  AVFormatContext* fmt_ctx;
  int v_index;
  int a_index;
} FileContext;

static FileContext inputFile;

static int open_decoder(AVCodecContext* codec_ctx)
{
  // Find a decoder by codec ID
  AVCodec* decoder = avcodec_find_decoder(codec_ctx->codec_id);
  if(decoder == NULL)
  {
    return -1;
  }

  // Open the codec using decoder
  if(avcodec_open2(codec_ctx, decoder, NULL) < 0)
  {
    return -2;
  }

  return 0;
}

static int open_input(const char* filename)
{
  unsigned int index;

  inputFile.fmt_ctx = NULL;
  inputFile.a_index = inputFile.v_index = -1;

  if(avformat_open_input(&inputFile.fmt_ctx, filename, NULL, NULL) < 0)
  {
    printf("Could not open input file %s\n", filename);
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
      if(open_decoder(codec_ctx) < 0)
      {
        break;
      }

      inputFile.v_index = index;
    }
    else if(codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && inputFile.a_index < 0)
    {
      if(open_decoder(codec_ctx) < 0)
      {
        break;
      }

      inputFile.a_index = index;
    }
  } // for

  if(inputFile.a_index < 0 && inputFile.a_index < 0)
  {
    printf("Failed to retrieve input stream information\n");
    return -3;
  }

  return 0;
}

static void release()
{
  if(inputFile.fmt_ctx != NULL)
  {
    unsigned int index;
    for(index = 0; index < inputFile.fmt_ctx->nb_streams; index++)
    {
      AVCodecContext* codec_ctx = inputFile.fmt_ctx->streams[index]->codec;
      if(index == inputFile.v_index || index == inputFile.a_index)
      {
        avcodec_close(codec_ctx);
      }
    }

    avformat_close_input(&inputFile.fmt_ctx);
  }
}

static int decode_packet(AVCodecContext* codec_ctx, AVPacket* pkt, AVFrame** frame, int* got_frame)
{
  int (*decode_func)(AVCodecContext *, AVFrame *, int *, const AVPacket *);
  int decoded_size;

  // Decide which is needed for decoding pakcet. 
  decode_func = (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 : avcodec_decode_audio4;
  decoded_size = decode_func(codec_ctx, *frame, got_frame, pkt);
  if(*got_frame)
  {
    // This adjust PTS/DTS automatically in frame.
    (*frame)->pts = av_frame_get_best_effort_timestamp(*frame);
  }

  return decoded_size;
}

int main(int argc, char* argv[])
{
  int ret;

  av_register_all();
  av_log_set_level(AV_LOG_DEBUG);

  if(argc < 2)
  {
    printf("usage : %s <input>\n", argv[0]);
    return 0;
  }

  if(open_input(argv[1]) < 0)
  {
    goto main_end;
  }

  // AVFrame is used to store raw frame, which is decoded from packet.
  AVFrame* decoded_frame = av_frame_alloc();
  if(decoded_frame == NULL) goto main_end;
  
  AVPacket pkt;
  int got_frame;
  
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

    AVStream* avStream = inputFile.fmt_ctx->streams[pkt.stream_index];
    AVCodecContext* codec_ctx = avStream->codec;
    got_frame = 0;

    av_packet_rescale_ts(&pkt, avStream->time_base, codec_ctx->time_base);

    ret = decode_packet(codec_ctx, &pkt, &decoded_frame, &got_frame);
    if(ret >= 0 && got_frame)
    {
      printf("-----------------------\n");
      if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
      {
        printf("Video : frame->width, height : %dx%d\n", 
          decoded_frame->width, decoded_frame->height);
        printf("Video : frame->sample_aspect_ratio : %d/%d\n", 
          decoded_frame->sample_aspect_ratio.num, decoded_frame->sample_aspect_ratio.den);
      }
      else
      {
        printf("Audio : frame->nb_samples : %d\n", 
          decoded_frame->nb_samples);
        printf("Audio : frame->channels : %d\n", 
          decoded_frame->channels);
      }

      av_frame_unref(decoded_frame);
    } // if

    av_free_packet(&pkt);
  } // while

  av_frame_free(&decoded_frame);

main_end:
  release();

  return 0;
}
```

```
AVFrame* av_frame_alloc	(void )	
Allocate an AVFrame and set its fields to default values.
The resulting struct must be freed using av_frame_free().
```


```
int	av_read_frame (AVFormatContext *s, AVPacket *pkt)
Return the next frame of a stream.

This function returns what is stored in the file, and does not validate that what is there are valid frames for the decoder. It will split what is stored in the file into frames and return one for each call. It will not omit invalid data between valid frames so as to give the decoder the maximum information possible for decoding.

If pkt->buf is NULL, then the packet is valid until the next av_read_frame() or until avformat_close_input(). Otherwise the packet is valid indefinitely. In both cases the packet must be freed with av_free_packet when it is no longer needed. For video, the packet contains exactly one frame. For audio, it contains an integer number of frames if each frame has a known fixed size (e.g. PCM or ADPCM data). If the audio frames have a variable size (e.g. MPEG audio), then it contains one frame.

pkt->pts, pkt->dts and pkt->duration are always set to correct values in AVStream.time_base units (and guessed if the format cannot provide them). pkt->pts can be AV_NOPTS_VALUE if the video format has B-frames, so it is better to rely on pkt->dts if you do not decompress the payload.
```


```
int	avcodec_decode_video2 (AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, const AVPacket *avpkt)
Decode the video frame of size avpkt->size from avpkt->data into picture.

Some decoders may support multiple frames in a single AVPacket, such decoders would then just decode the first frame.

Warning
The input buffer must be AV_INPUT_BUFFER_PADDING_SIZE larger than the actual read bytes because some optimized bitstream readers read 32 or 64 bits at once and could read over the end.
The end of the input buffer buf should be set to 0 to ensure that no overreading happens for damaged MPEG streams.
Note
Codecs which have the AV_CODEC_CAP_DELAY capability set have a delay between input and output, these need to be fed with avpkt->data=NULL, avpkt->size=0 at the end to return the remaining frames.
The AVCodecContext MUST have been opened with avcodec_open2() before packets may be fed to the decoder.
```

```
int avcodec_decode_audio4 (AVCodecContext *avctx, AVFrame *frame, int *got_frame_ptr, const AVPacket *avpkt)
Decode the audio frame of size avpkt->size from avpkt->data into frame.

Some decoders may support multiple frames in a single AVPacket. Such decoders would then just decode the first frame and the return value would be less than the packet size. In this case, avcodec_decode_audio4 has to be called again with an AVPacket containing the remaining data in order to decode the second frame, etc... Even if no frames are returned, the packet needs to be fed to the decoder with remaining data until it is completely consumed or an error occurs.

Some decoders (those marked with AV_CODEC_CAP_DELAY) have a delay between input and output. This means that for some packets they will not immediately produce decoded output and need to be flushed at the end of decoding to get all the decoded data. Flushing is done by calling this function with packets with avpkt->data set to NULL and avpkt->size set to 0 until it stops returning samples. It is safe to flush even those decoders that are not marked with AV_CODEC_CAP_DELAY, then no samples will be returned.

Warning
The input buffer, avpkt->data must be AV_INPUT_BUFFER_PADDING_SIZE larger than the actual read bytes because some optimized bitstream readers read 32 or 64 bits at once and could read over the end.
Note
The AVCodecContext MUST have been opened with avcodec_open2() before packets may be fed to the decoder.
```
