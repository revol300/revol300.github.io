---
layout: post
title:  "FFmpeg을 한번 써보자 (2)"
date:   2020-04-29
comments: true
excerpt: "ffmpeg tutorial02"
categories: ffmpeg
tag:
- c
- multimedia 
- ffmpeg 
---

# multimedia file을 열고 demuxing까지 해보자!

```c++
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

typedef struct _FileContext {
  AVFormatContext* fmt_ctx;
  int v_index;
  int a_index;
} FileContext;

static FileContext input_ctx;

static int open_input(const char* filename) {
  unsigned int index;
  input_ctx.fmt_ctx = NULL;
  input_ctx.v_index = input_ctx.a_index = -1;

  if(avformat_open_input(&input_ctx.fmt_ctx, filename, NULL, NULL) < 0) {
    printf("Could not open input file %s\n", filename);
    return -1;
  }
  if(avformat_find_stream_info(input_ctx.fmt_ctx, NULL) < 0) {
    printf("Failed to retrieve input stream information\n");
    return -2;
  }

  for(index = 0; index < input_ctx.fmt_ctx->nb_streams; index++) {
    AVCodecContext* codec_ctx = input_ctx.fmt_ctx->streams[index]->codec;
    if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && input_ctx.v_index < 0)
    {
      input_ctx.v_index = index;
    }
    else if(codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && input_ctx.a_index < 0)
    {
      input_ctx.a_index = index;
    }
  } // for

  if(input_ctx.v_index < 0 && input_ctx.a_index < 0) {
    printf("Failed to retrieve input stream information\n");
    return -3;
  }

  return 0;
}

static void release() {
  if(input_ctx.fmt_ctx != NULL)
  {
    avformat_close_input(&input_ctx.fmt_ctx);
  }
}

int main(int argc, char* argv[]) {
  int ret;

  av_register_all();
  av_log_set_level(AV_LOG_DEBUG);

  if(argc < 2) {
    printf("usage : %s <input>\n", argv[0]);
    return 0;
  }

  if(open_input(argv[1]) < 0) {
    goto main_end;
  }

  // AVPacket is used to store packed stream data.
  AVPacket pkt;

  while(1) {
    ret = av_read_frame(input_ctx.fmt_ctx, &pkt);
    if(ret == AVERROR_EOF)
    {
      // No more packets to read.
      printf("End of frame\n");
      break;
    }

    if(pkt.stream_index == input_ctx.v_index)
    {
      printf("Video packet\n");
    }
    else if(pkt.stream_index == input_ctx.a_index)
    {
      printf("Audio packet\n");
    }

    av_free_packet(&pkt);
  } // while

main_end:
  release();
  return 0;
}
```
먼저 main 함수부터 살펴보자
```c++
int main(int argc, char* argv[]) {
  int ret;

  av_register_all();
  av_log_set_level(AV_LOG_DEBUG);

  if(argc < 2) {
    printf("usage : %s <input>\n", argv[0]);
    return 0;
  }

  if(open_input(argv[1]) < 0) {
    goto main_end;
  }

  // AVPacket is used to store packed stream data.
  AVPacket pkt;

  while(1) {
    ret = av_read_frame(input_ctx.fmt_ctx, &pkt);
    if(ret == AVERROR_EOF)
    {
      // No more packets to read.
      printf("End of frame\n");
      break;
    }

    if(pkt.stream_index == input_ctx.v_index)
    {
      printf("Video packet\n");
    }
    else if(pkt.stream_index == input_ctx.a_index)
    {
      printf("Audio packet\n");
    }

    av_free_packet(&pkt);
  } // while

main_end:
  release();
  return 0;
}
```
지난번 튜토리얼에서 파일을 열고 정보를 읽어들이는 과정을 보았는데, 한발 더 나아가서 이제 실제로 demuxing을 한번 해보자. 우선 main함수를 보면 파일 이름을 통해서 파일을 여는데까지는 동일하다. 약간 다른 점은 static 함수로 수행되는 open_input인데, open_input의 구현은 아래와 같다.

```c++
static int open_input(const char* filename) {
  unsigned int index;

  input_ctx.fmt_ctx = NULL;
  input_ctx.v_index = input_ctx.a_index = -1;

  if(avformat_open_input(&input_ctx.fmt_ctx, filename, NULL, NULL) < 0) {
    printf("Could not open input file %s\n", filename);
    return -1;
  }

  if(avformat_find_stream_info(input_ctx.fmt_ctx, NULL) < 0) {
    printf("Failed to retrieve input stream information\n");
    return -2;
  }

  for(index = 0; index < input_ctx.fmt_ctx->nb_streams; index++) {
    AVCodecContext* codec_ctx = input_ctx.fmt_ctx->streams[index]->codec;
    if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && input_ctx.v_index < 0)
    {
      input_ctx.v_index = index;
    }
    else if(codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && input_ctx.a_index < 0)
    {
      input_ctx.a_index = index;
    }
  } // for

  if(input_ctx.v_index < 0 && input_ctx.a_index < 0)
  {
    printf("Failed to retrieve input stream information\n");
    return -3;
  }

  return 0;
}
```
find_stream_info 까지의 과정은 동일하지만 이번에는 demuxing을 위해서 stream별 코덱정보가 필요하기 때문에 이를 얻어주는 과정이 추가 되었다. input_ctx.fmt_ctx에서 각stream별 index정보를 통해 어떤 것이 audio stream이고 video stream인지에 대한 정보를 input_ctx 구조체에 넣어준다.
이제 파일에서 data를 얻어오자. 이는 다음 함수를 통해서 이루어진다.
```
int	av_read_frame (AVFormatContext *s, AVPacket *pkt) : Return the next frame of a stream.

This function returns what is stored in the file, and does not validate that what is there are valid frames for the decoder. It will split what is stored in the file into frames and return one for each call. It will not omit invalid data between valid frames so as to give the decoder the maximum information possible for decoding.

If pkt->buf is NULL, then the packet is valid until the next av_read_frame() or until avformat_close_input(). Otherwise the packet is valid indefinitely. In both cases the packet must be freed with av_free_packet when it is no longer needed. For video, the packet contains exactly one frame. For audio, it contains an integer number of frames if each frame has a known fixed size (e.g. PCM or ADPCM data). If the audio frames have a variable size (e.g. MPEG audio), then it contains one frame.

pkt->pts, pkt->dts and pkt->duration are always set to correct values in AVStream.time_base units (and guessed if the format cannot provide them). pkt->pts can be AV_NOPTS_VALUE if the video format has B-frames, so it is better to rely on pkt->dts if you do not decompress the payload. 
```
간단하게 말그대로 file에서 한 frame을 읽어서 pkt에 넣어주는 역할을 하는 함수이다.
pkt을 읽고난 뒤에넌 buffer를 비우고 새로운 frame을 읽어야하기 때문에 `av_free_packet` 함수를 통해서 packet의 buffer를 비워준다.이후 모든 file을 끝까지 읽었다면 release함수의 `avformat_close_input(&input_ctx.fmt_ctx)`를 통해서 파일을 닫아주도록하자.

다음 튜토리얼에서는 demuxing된 stream을 remuxing해서 다른 포멧의 멀티미디어 파일을 만들어보도록하자.
