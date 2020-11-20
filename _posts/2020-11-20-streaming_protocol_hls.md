---
layout: post
title:  "Streaming Service - HLS"
date:   2020-04-29
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

client는 decryption key를 통해서 user 
