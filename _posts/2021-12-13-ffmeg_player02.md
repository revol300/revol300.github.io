---
layout: post
title:  "FFmpeg을 이용한 간단한 player 구현 (FFmpeg tutorial 2)"
date:   2021-12-13
comments: true
project: true
excerpt: "Simple player with FFmpeg"
categories: ffmpeg
tag:
- c
- multimedia 
- ffmpeg 
---

## Demuxer interface
Demuxing 작업은 파일을 열고 압축된 데이터 패킷을 뽑아오는 과정이다.
즉 input으로 파일 경로를 주면, Demuxer는 이에 맞게 순차적으로 audio, video 패킷을 output으로 준다.
이에 따라 구현할 Demuxer의 interface는 다음과 같다.

demuxer.h
```cpp
class Demuxer {
public:
  explicit Demuxer(const std::string &file_path);
  ~Demuxer();
  int init();
  bool getPacket(std::shared_ptr<AVPacket> &packet);
private:
  std::string file_path_;
};
```

AVPacet은 ffmpeg에 정의된 데이터 패킷 object라고 보면 된다.

아직 완성되지는 않았지만 Demuxer를 이용하여 packet을 하나 가져오는 코드를 써보면 다음과 같이 쓰여진다

demuxer_test.cc
```cpp
#include <iostream>
#include <memory>

#include "demuxer/demuxer.h"

using std::cout;
using std::endl;
using std::make_shared;

int main(int argc, char *argv[]) {
  av_register_all(); //@NOTE: For FFmpeg version < 4.0
  if (argc < 2) {
    printf("usage : %s <input>\n", argv[0]);
    return 0;
  }
  std::string file_path = argv[1];
  std::shared_ptr<Demuxer> demuxer = make_shared<Demuxer>(file_path);
  demuxer->init();
  std::shared_ptr<AVPacket> packet;
  demuxer->getPacket(packet);
  return 0;
}
```
av_register_all은 FFmpeg을 사용하기 전에 호출해야하는 함수로 FFmpeg version 4.0이상부터는 호출할 필요가 없다.

## Demuxing 구현

이제 demuxer.h의 함수를 각각 구현해보자

먼저 생성자에서 file_path를 초기화한다.
```cpp
Demuxer::Demuxer(const std::string &file_path)
    : file_path_(file_path) {}
```

Demuxer의 init 함수에서는 FFmpeg의 avformat_open_input을 통해 file_path의 파일의 정보를 읽어온다. 

```cpp
int Demuxer::init() {
  AVFormatContext *fmt_ctx = nullptr;
  int ret = avformat_open_input(&fmt_ctx, file_path_.c_str(), NULL, NULL);
  if (ret < 0) {
    char error_msg[256];
    av_make_error_string(error_msg, 256, ret);
    cout << "Could not open input file " << file_path_
         << " error_msg : " << error_msg << endl;
    //@NOTE error_code needed
    return -1;
  }
  fmt_ctx_ = std::shared_ptr<AVFormatContext>(fmt_ctx);
}
```
avformat_open_input 함수를 통해서 AVFormatContext를 초기화 하게 되는데, 파일에 대한 packet정보를 가져올 때마다 사용되는 값이므로 Demuxer의 멤버 변수로 저장되어야한다. demuxer.h 에 다음과 같이 변수를 추가해주자.

demuxer.h
```cpp
class Demuxer {
...
private:
+ std::shared_ptr<AVFormatContext> fmt_ctx_;
  std::string file_path_;
...
};
```

이제 getPacket을 구현해보자. getPacket에서는 들어온 packet에 파일로부터 읽어들인 data를 채워주는 작업이 이뤄진다. av_read_frame 함수를 통해서 이를 수행할 수 있다.

demuxer.cc
```cpp
bool Demuxer::getPacket(std::shared_ptr<AVPacket> &packet) {
  AVPacket *media_packet = av_packet_alloc();
  if (av_read_frame(fmt_ctx_.get(), media_packet) >= 0) {
    packet = std::shared_ptr<AVPacket>(media_packet, [](AVPacket *packet) {
      av_packet_free(&packet);
      cout << "packet free" << endl;
    });
    return true;
  } else {
    cout << "Invalid Packet" << endl;
    av_packet_free(&media_packet);
    return false;
  }
}
```
이제 Demuxer를 통해 동영상 파일을 읽어들이고 packet을 가져올 수 있다!

## video, audio packet의 구분

getPacket을 이용해서 packet을 가져오긴 했지만 이게 video인지 audio인지는 모르는 상황이다. 다행히 AVPacket 구조체의 stream_index 값을 통해 audio인지 video인지 구분이 가능하다. 다만 어떤 stream_index가 어떤 타입의 패킷을 가르키는지는 알 수 없기 때문에 이 정보는 AVFormatContext에서 가져와야 한다.

다만 avformat_open_input을 통해 AVFormatContext에 값을 채워놓기는 했지만 파일 내부의 각 stream에 대한 정보가 동영상 컨테이너의 구조에 따라 업데이트 되지 않았을 수 있다. FFmpeg의 avformat_find_stream_info 함수는 각 data stream에 대한 정보를 업데이트 해준다. 해당 함수가 수행된 뒤에는 av_find_best_stream함수를 통해 default video 및 audio의 stream_index를 알 수 있다. 이를 코드에 반영해보자

demuxer.cc
```cpp
int Demuxer::init() {
  ...
  fmt_ctx_ = std::shared_ptr<AVFormatContext>(fmt_ctx);
+ ret = avformat_find_stream_info(fmt_ctx_.get(), NULL);
+ if (ret < 0) {
+   cout << "Failed to retrieve input stream information : " << endl;
+   return -2;
+ }
+ video_index_ =
+     av_find_best_stream(fmt_ctx_.get(), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
+ if (video_index_ == AVERROR_STREAM_NOT_FOUND)
+   video_index_ = -1;
+ if (video_index_ < 0) {
+   cout << "Failed to retrieve input stream information" << endl;
+   return -3;
+ }
+
+ audio_index_ =
+     av_find_best_stream(fmt_ctx_.get(), AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
+ if (audio_index_ == AVERROR_STREAM_NOT_FOUND)
+   audio_index_ = -1;
+
+ if (audio_index_ < 0) {
+   cout << "Failed to retrieve input stream information" << endl;
+   return -3;
+ }
}
```

외부에서 Demuxer모듈을 통해서 video_index_ 및 audio_index_의 값을 사용해야 하기 때문에 추가적인 함수도 뚫어주자.

demuxer.h
```cpp
class Demuxer {
...
  bool getPacket(std::shared_ptr<AVPacket> &packet);
+ int getVideoIndex() { return video_index_; }
+ int getAudioIndex() { return audio_index_; }

...
  std::string file_path_;
+ int video_index_;
+ int audio_index_;
};
```

## 전체 코드
./demuxer/demuxer.h
```cpp
#ifndef DEMUXER_DEMUXER_H_
#define DEMUXER_DEMUXER_H_

extern "C" {
#include <libavformat/avformat.h>
}

#include <memory>
#include <string>

class Demuxer {
public:
  explicit Demuxer(const std::string &file_path);
  ~Demuxer();
  int init();
  bool getPacket(std::shared_ptr<AVPacket> &packet);
  int getVideoIndex() { return video_index_; }
  int getAudioIndex() { return audio_index_; }

private:
  std::shared_ptr<AVFormatContext> fmt_ctx_;
  std::string file_path_;
  int video_index_;
  int audio_index_;
};

#endif // DEMUXER_DEMUXER_H_
```

./demuxer/demuxer.cc
```cpp
#include "demuxer/demuxer.h"

#include <iostream>
#include <memory>

using std::cout;
using std::endl;
using std::shared_ptr;

Demuxer::Demuxer(const std::string &file_path)
    : file_path_(file_path), video_index_(-1), audio_index_(-1) {}

Demuxer::~Demuxer() {}

int Demuxer::init() {
  AVFormatContext *fmt_ctx = nullptr;
  int ret = avformat_open_input(&fmt_ctx, file_path_.c_str(), NULL, NULL);
  if (ret < 0) {
    char error_msg[256];
    av_make_error_string(error_msg, 256, ret);
    cout << "Could not open input file " << file_path_
         << " error_msg : " << error_msg << endl;
    //@NOTE error_code needed
    return -1;
  }
  fmt_ctx_ = std::shared_ptr<AVFormatContext>(fmt_ctx);
  ret = avformat_find_stream_info(fmt_ctx_.get(), NULL);
  if (ret < 0) {
    cout << "Failed to retrieve input stream information : " << endl;
    return -2;
  }
  video_index_ =
      av_find_best_stream(fmt_ctx_.get(), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  if (video_index_ == AVERROR_STREAM_NOT_FOUND)
    video_index_ = -1;
  if (video_index_ < 0) {
    cout << "Failed to retrieve input stream information" << endl;
    return -3;
  }

  audio_index_ =
      av_find_best_stream(fmt_ctx_.get(), AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
  if (audio_index_ == AVERROR_STREAM_NOT_FOUND)
    audio_index_ = -1;

  if (audio_index_ < 0) {
    cout << "Failed to retrieve input stream information" << endl;
    return -3;
  }
}

bool Demuxer::getPacket(std::shared_ptr<AVPacket> &packet) {
  AVPacket *media_packet = av_packet_alloc();
  if (av_read_frame(fmt_ctx_.get(), media_packet) >= 0) {
    packet = std::shared_ptr<AVPacket>(media_packet, [](AVPacket *packet) {
      av_packet_free(&packet);
      cout << "packet free" << endl;
    });
    return true;
  } else {
    cout << "Invalid Packet" << endl;
    av_packet_free(&media_packet);
    return false;
  }
}
```

./demuxer/demuxer_test.cc
```cpp
#include <iostream>
#include <memory>

#include "demuxer/demuxer.h"

using std::cout;
using std::endl;
using std::make_shared;

int main(int argc, char *argv[]) {
  av_register_all(); //@NOTE: For FFmpeg version < 4.0
  if (argc < 2) {
    printf("usage : %s <input>\n", argv[0]);
    return 0;
  }
  std::string file_path = argv[1];
  std::shared_ptr<Demuxer> demuxer = make_shared<Demuxer>(file_path);
  demuxer->init();
  std::shared_ptr<AVPacket> packet;
  demuxer->getPacket(packet);
  return 0;
}
```
이제 Demuxer를 통해서 파일을 읽고 압축된 data packet을 얻어올 수 있다!


다음 포스팅에서는 packet을 raw data로 decoding하는 Decoder 구현 과정을 써보려고 한다.
