---
layout: post
title:  "Streaming Service - HLS"
date:   2020-11-20
comments: true
excerpt: "HLS (HTTP Live Streaming)"
categories: multimedia
tag:
- multimedia 
---
# HTTP Live Streaming이란?
HTTP Live Streaming(HLS)란 Apple에서 audio와 video 스트리밍을 위해 HTTP를 이용햐여 만든 프로토콜이다. => [HLS 스펙 문서](https://tools.ietf.org/html/draft-pantos-hls-rfc8216bis-08)


Apple 홈페이지의 설명에 따르면 다음과 같은 기능을 지원한다고 한다
- 라이브 방송 및 사전 녹화된 컨텐츠 (VOD)
- 다중 비트레이트 stream 지원과 네트워크 bandwidth에 따라 상황에 맞는 stream을 사용하도록 지원
- 미디어 암호화 및 사용자 인증
아래 그림은 HTTP Live Stream에 대한 구조이다
![HLS](https://docs-assets.developer.apple.com/published/88e87744a3/de18e941-81de-482f-843d-834a4dd3aa71.png)

# HLS의 구조
HTTP Live Streaming은 크게 server ,distribution component, 그리고 client software 세부분으로 나눌수 있다.  

일반적으로 데이터 흐름은 다음과 같이 이루어진다.
1. 하드웨어가 비디오 및 오디오 입력을 받아서 AC-3나 HEVC코덱을 이용하여 인코딩한다.

2. 이를 segmenter가 일련의 쪼개진 media file의 형태로 만들어서 web server에 놓는다.

3. 파일들에 대한 정보는 indexing해서 file로 따로 만들어 놓는다.

4. web server에서는 이 index file을 publish하게 되고 client는 이 index file을 열어서 list에 있는 파일을 web server에 요청한다. 그리고 이 파일을 통해서 영상 재생이 이루어진다.

## Server Component
Server에서 이와 관련하여 맡는 역할은 media input을 받아서 encoding 한뒤 이를 전송에 용이한 형태로 캡슐화하는 것이다. 이와 관련된 software는 Apple에서 제공하는 media stream segmenter나 서드 파티 솔루션을 사용할 수 있다.

## Distribution Component
distribution system은 web server 혹은 web-caching system으로 media file과 index file을 HTTP를 통해서 전송하는 역할을 맡는다. 특별히 설정할 내용은 거의 없다.

## Client Software
index file을 통해서 적합한 media file을 web server에 요청하고 다운 받아진 resource를 연속적인 stream으로 재생할 수 있게 재구성 해야하는 역할이다. 

index file에는 지금 다운받을 수 있는 media file, decryption key 그리고 대체 할 수 있는 stream에 대한 정보를 포함하고 있다. client는 index file을 통해 원하는 media file을 요청하고 각 파일에 있는 연속적인 stream segment를 다운받는다. 충분한 양의 데이터를 다운받으면 클라이언트는 이를 통해서 stream에 대한 정보를 사용자에게 보여준다.
이런 프로세스는 client가 EXT-X-ENDLIST tag를 index file에서 보기 전까지 반복된다. 해당 태그가 보이지 않으면 client는 live streaming이 계속 진행중이라 판단하고 index file을 주기적으로 호출한다. 

# Deploying a Basic HTTP Live Stream 
[원본 문서](https://developer.apple.com/documentation/http_live_streaming/deploying_a_basic_http_live_stream)
그러면 HLS에 대해서는 대략적인 설명을 했으므로 실제로 HLS를 통해 영상을 재생해보자

일단 HLS를 이용하려면 3가지가 필요하다. 

1. Server Component : segmenet로 나눠진 파일 생성

2. Distribution Component : 이렇게 만들어진 파일을 web server 나 CDN으로 client에게 제공

3. Client Software : hls프로토콜에 맞추어 영상을 사용자에게 보여주는 software 

1,2는 이미 segement파일로 만들어서 client에게 제공해주고 있는 link를 이용하자 => http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8

3번은 brower를 사용하자 safari에서는 video 태그 자체에서 hls를 지원하기 때문에 별도로 추가할 사항이 없지만 다른 브라우저의 경우에는 hls기능 지원을 위해 hls.js가 필요할 수 있다.

이것이 포함된 HTML page를 생성해보자

아래는 [hls.js](https://github.com/video-dev/hls.js) 페이지에서 받아온 코드이다.

```html
<html>
<head>
  <meta content="text/html;charset=utf-8" http-equiv="Content-Type">
  <title>HTTP Live Streaming Example</title>
</head>
<body>
  <script src="https://cdn.jsdelivr.net/npm/hls.js@latest"></script>
  <!-- Or if you want a more recent alpha version -->
  <!-- <script src="https://cdn.jsdelivr.net/npm/hls.js@alpha"></script> -->
  <video id="video" controls autoplay></video>
  <script>
    var video = document.getElementById('video');
    var videoSrc = 'https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8';
    if (Hls.isSupported()) {
    var hls = new Hls();
    hls.loadSource(videoSrc);
    hls.attachMedia(video);
    hls.on(Hls.Events.MANIFEST_PARSED, function() {
      video.play();
    });
  }
  // hls.js is not supported on platforms that do not have Media Source
  // Extensions (MSE) enabled.
  //
  // When the browser has built-in HLS support (check using `canPlayType`),
  // we can provide an HLS manifest (i.e. .m3u8 URL) directly to the video
  // element through the `src` property. This is using the built-in support
  // of the plain video element, without using hls.js.
  //
  // Note: it would be more normal to wait on the 'canplay' event below however
  // on Safari (where you are most likely to find built-in HLS support) the
  // video.src URL must be on the user-driven white-list before a 'canplay'
  // event will be emitted; the last video event that can be reliably
  // listened-for when the URL is not on the white-list is 'loadedmetadata'.
  else if (video.canPlayType('application/vnd.apple.mpegurl')) {
    video.src = videoSrc;
    video.addEventListener('loadedmetadata', function() {
      video.play();
    });
  }
  </script>
</body>
</html>
```
# HLS의 보안
HLS에서는 컨텐츠 파일(ex. ts)을 encrypt하여 전송하는 방식을 지원하고 있다. 사용 권한의 제어 기능이나 키 관리 기능은 제공하지 않는다. 한편  encryption에 사용되는 key를 주기적으로 변경하는 기능도 제공을 하고 있으며 해당 키의 전달은 HTTP 및 HTTPS를 통해서 전달할 수 있다. 키값이기 때문에 HTTPS 사용이 권장된다.
key file은 encrypte된 media를 decode하기위해 initialization vertor를 필요로 한다 
