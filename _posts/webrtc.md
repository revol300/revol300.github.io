webrtcforthecurious.com
#SDP :Session Description Protocol
P2P 연결에서 사용자 간의 미디어 타입과 포맷에 대해 협상하기 위해 개발된 프로토콜
## JSON syntax
{
  key : value
}

## SDP syntax
key=value

## WebRTC에서 쓰이는 SDP 키 값
- v : Version
- o : Origin, contains a unique ID userful for renegotiations
- s : Session Name
- t : Timing
- m : Media Description
- a : Attribute
- c : Connection Data

#ICE : Interactive Connectivity Establishment
피어 간 연결을 책임지는 framework

#RTP : Real-time Transport Protocol && RTCP : Real-time Transport Control Protocol
RTP는 멀티미디어 데이터 전송을 위한 프로토콜
RTCP는 RTP에 대한 컨트롤을 맡고 있는 프로토콜로써 데이터 전송 통계나 packet loss등 다양한 통계정보를 제공하기도 한다.
RTP연결 하나당 RTCP연결하나가 필요하다

#SCTP : Stream Control Transimssion Protocol
RTP와 역할은 비슷하지만 멀티미디어 데이터를 않는다는 것에서 다르다
ex.) Real-time network games, Game player action events, Asset exchange, Text chat

#DTLS : Datagram Transport Layer Security && SRTP : Secure Real-time Transport Protocol
Security를 담당

#NAT Traversal
A New Method for Symmetric NAT Traversal in UDP and TCP
[https://alnova2.tistory.com/1110](https://alnova2.tistory.com/1110)

mungesdp (webRTC protocol을 써볼 수 있다)
