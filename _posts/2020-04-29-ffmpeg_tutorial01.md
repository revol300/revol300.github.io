---
layout: post
title:  "FFmpeg을 한번 써보자 (1)"
date:   2020-04-29
comments: true
excerpt: "ffmpeg tutorial01"
categories: ffmpeg
tag:
- c
- multimedia 
- ffmpeg 
---

# 다룰 내용은 ?
[FFmpeg 라이브러리](http://www.yes24.com/Product/Goods/20365557)을 읽고 난뒤 예제를 그냥 읽기만 하는 것으로는 익숙해지기가 쉽지 않아서 해당 내용을 정리해보고자한다.

* 코드 출처는 [여기](https://github.com/sorrowhill/FFmpegTutorial)

# Tutorial1 : multimedia file의 정보를 얻어와보자

우선 전체 코드를 함 보자!!

```c++
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

static AVFormatContext* fmt_ctx = NULL;

int main(int argc, char* argv[])
{
  unsigned int index;

  av_register_all();

  // Print debug log in library level.
  av_log_set_level(AV_LOG_DEBUG);

  if(argc < 2)
  {
    printf("usage : %s <input>\n", argv[0]);
    return 0;
  }

  // Get fmt_ctx from given file path. 
  if(avformat_open_input(&fmt_ctx, argv[1], NULL, NULL) < 0)
  {
    printf("Could not open input file %s\n", argv[1]);
    return -1;
  }

  // Find stream information from given fmt_ctx.
  if(avformat_find_stream_info(fmt_ctx, NULL) < 0)
  {
    printf("Failed to retrieve input stream information\n");
    return -2;
  }

  // fmt_ctx->nb_streams : number of total streams in video file.
  for(index = 0; index < fmt_ctx->nb_streams; index++)
  {
    AVCodecContext* avCodecContext = fmt_ctx->streams[index]->codec;
    if(avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      printf("------- Video info -------\n");
      printf("codec_id : %d\n", avCodecContext->codec_id);
      printf("bitrate : %d\n", avCodecContext->bit_rate);
      printf("width : %d / height : %d\n", avCodecContext->width, avCodecContext->height);
    }
    else if(avCodecContext->codec_type == AVMEDIA_TYPE_AUDIO)
    {
      printf("------- Audio info -------\n");
      printf("codec_id : %d\n", avCodecContext->codec_id);
      printf("bitrate : %d\n", avCodecContext->bit_rate);
      printf("sample_rate : %d\n", avCodecContext->sample_rate);
      printf("number of channels : %d\n", avCodecContext->channels);
    }
  } // for

  if(fmt_ctx != NULL)
  {
    avformat_close_input(&fmt_ctx);
  }

  return 0;
}
```

도통 뭔소린지 알 수가 없다. 그러므로 든든한 [reference](http://www.ffmpeg.org/doxygen/3.0/) 와 함께 찬찬히 하나 하나 알아가보자.

```c++
unsigned int index;

av_register_all();

// Print debug log in library level.
av_log_set_level(AV_LOG_DEBUG);

```

우선 처음으로 만나는 함수는 av_register_all() 로 이 함수가 하는 일은 다음과 같다고 한다.
```
av_register_all() : Initialize libavformat and register all muxers, demuxers and protocols
If you do not call this function, then you can select exactly which format you want to support.
```
그러니까 얘를 실행함으로써 FFmpeg에서 사용할 수 있는 모든 muxer와 demuxer, incoder, decoder목록을 초기화한다. 최신 버전에서는 deprecated되었고 굳이 쓸필요가 없다고 한다.


```c++
if(argc < 2)
{
  printf("usage : %s <input>\n", argv[0]);
  return 0;
}

// Get fmt_ctx from given file path. 

if(avformat_open_input(&fmt_ctx, argv[1], NULL, NULL) < 0)
{
  printf("Could not open input file %s\n", argv[1]);
  return -1;
}
```

그런 뒤에 인자로 받은 path를 통해서 file을 열어줘한다

```
avformat_open_input (AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options)
Open an input stream and read the header.
The codecs are not opened. The stream must be closed with avformat_close_input().
```
단순하게 filename에 해당하는 파일을 열어서 header를 읽고 input stream을 열어주는 함수이다.

```c++
// Find stream information from given fmt_ctx.
if(avformat_find_stream_info(fmt_ctx, NULL) < 0)
{
  printf("Failed to retrieve input stream information\n");
  return -2;
}
```

```
avformat_find_stream_info (AVFormatContext *ic, AVDictionary **options) : Read packet of a media file to get stream information. This is useful for file format with no header such as MPEG. This function also computes the real framerate in case of MPEG-2 repeat frame mode. The logical file position is not changed by this function; examined packets may be buffered for laterprocessing.
```
이제 열어놓은 파일에 대해서 stream 정보를 가져온다. 

AVFormatContext의 구조체 정보는 [여기](https://ffmpeg.org/doxygen/3.3/structAVFormatContext.html) 에서 볼 수 있다.
이제 필요한 정보는 다 받아왔으므로 아래와 같이 출려해주면,

```c++
// fmt_ctx->nb_streams : number of total streams in video file.
for(index = 0; index < fmt_ctx->nb_streams; index++)
{
  AVCodecContext* avCodecContext = fmt_ctx->streams[index]->codec;
  if(avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO)
  {
    printf("------- Video info -------\n");
    printf("codec_id : %d\n", avCodecContext->codec_id);
    printf("bitrate : %d\n", avCodecContext->bit_rate);
    printf("width : %d / height : %d\n", avCodecContext->width, avCodecContext->height);
  }
  else if(avCodecContext->codec_type == AVMEDIA_TYPE_AUDIO)
  {
    printf("------- Audio info -------\n");
    printf("codec_id : %d\n", avCodecContext->codec_id);
    printf("bitrate : %d\n", avCodecContext->bit_rate);
    printf("sample_rate : %d\n", avCodecContext->sample_rate);
    printf("number of channels : %d\n", avCodecContext->channels);
  }
}
```
Video 정보와 Audio 정보를 가져 올 수 있다.

마지막으로 깔끔하게 정리해주자

```c++
if(fmt_ctx != NULL)
{
  avformat_close_input(&fmt_ctx);
}
```

```
avformat_close_input (AVFormatContext** s) : Close an opened input AVFormatContext. Free it and all its contents and set *s to NULL
```


# Tutorial2에서는 FFmpeg을 file의 데이터를 demuxing 하는 과정에 대해서 알아보자
