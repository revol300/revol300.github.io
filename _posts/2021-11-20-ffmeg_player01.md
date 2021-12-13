---
layout: post
title:  "FFmpeg을 이용한 간단한 player 구현 (FFmpeg tutorial 1)"
date:   2021-11-29
comments: true
project: true
excerpt: "Simple player with FFmpeg"
categories: ffmpeg
tag:
- c
- multimedia 
- ffmpeg 
---

## Introudction

본 글은 C++로 간단한 player를 만들고 해당 내용을 정리하고자 쓰였습니다. player의 구현에는 다음과 같은 라이브러리들이 사용됩니다.


- [FFmpeg](https://ffmpeg.org)은 오디오 및 비디오의 압축 컨버팅 녹화 등 다양한 기능을 모아놓은 크로스 플랫폼 라이브러리입니다. player 구현에서는 FFmpeg의 demuxing, decoding 기능을 이용하여 동영상 파일로부터 오디오 비디오 패킷을 가져와 재생가능한 형태로 바꾸는 과정을 진행합니다

- [SDL2](https://www.libsdl.org/index.php)은 디스플레이, 키보드, 마우스, 스피커 등의 device에 손쉽게 접근할 수 있도록 도와주는 크로스 플랫폼 라이브러리입니다. SDL2는 FFmpeg으로 파일로부터 만들어낸 raw video data 및 raw audio data를 출력하고, 키보드 입력을 통해 player의 seek 및 pause 기능을 구현하기 위해 사용합니다.

** 전체 코드는 [여기](https://github.com/revol300/SimplePlayerWithFFmpeg)서 확인해보실 수 있습니다. **

## Player 동작 과정

![player_intro](/assets/img/postImages/player_intro.jpg)


시작하기에 앞서 프로그램이 동작하는 방식에 대해서 정리해보겠습니다. 미디어 파일은 압축된 video 및 audio data를 패킷이라는 단위로 가지고 있습니다. 파일에서 audio 및 video 데이터를 재생하기 위해서는 먼저 파일에서 video 및 audio packet을 분리하는 과정이 필요합니다. 이를 Demuxing이라고 합니다.
![player_demuxing](/assets/img/postImages/player_demuxing.jpg)


Demuxing을 통해서 가져온 audio, video packet은 현재 압축이 되어 있는 상태입니다. 이렇게 압축되어 있는 상태로는 오디오 출력 및 비디오 출력이 불가능하므로 재생을 위해서는 해당 packet의 압축을 풀어주는 작업이 필요합니다. 이 작업을 Decoding이라고 합니다.
![player_decoding](/assets/img/postImages/player_decoding.jpg)


한편 압축을 풀어준  video data, audio data의 형태가 현재 장치에 맞지 않을 수가 있습니다. 이러한 경우 audio, video data를 장치에 맞게 변환하는 과정이 필요합니다. 이를 Converting 이라고 합니다.
![player_converting](/assets/img/postImages/player_converting.jpg)


마지막으로 출력가능한 형태의 audio, video data를 장치로 전달해주는 과정을 통해 video 및 audio 출력이 일어나게 되는데 이를 Rendering이라고 합니다.
![player_rendering](/assets/img/postImages/player_rendering.jpg)


구현 과정 또한 순서대로 Demuxing, Decoding, Converting, Rendering을 구현하면서 진행됩니다.


다음 포스팅에서는 Demuxing과정을 구현해보고자합니다.
