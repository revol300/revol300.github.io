---
layout: post
title:  "CABAC vs CAVLC (1)"
date:   2021-03-17
comments: true
excerpt: CABAC 
categories: multimedia
tag:
- multimedia 
---
#Introduction
H.264에서는 코덱 사용목적에 따라 프로파일을 나누고, 어떤 프로파일을 사용하는지에 따라 다른 방법을 통해 encoding 및 decoding을 지원한다. 특히 그중에 차이를 보이는 것중에 하나가 메크로블록의 DCT coefficient를 압축하는 과정이다. H.264에는 프로파일의 종류에 따라 CAVLC(Context-adaptive variable-length coding) 혹은 CABAC(Context-adaptive binary arithmetic coding)을 이용한다. 간단히 장단을 비교하자면 CAVLC는 압축효율이 좀 떨어지지만 연산량이 적고 CABAC의 경우에는 연산량이 많지만 압축효율이 좋다. 그러한 연유로 자연스레 컴퓨터의 성능이 좋아지면서 H.265에서는 둘중에 하나를 선택하는 것이 아닌 CABAC을 사용한다고 한다. 이제 CAVLC 및 CABAC가 어떤 방식으로 압축을 진행하는지에 대해서 알아보자

#CAVLC (Context-adaptive variable-length coding)
다음 [링크](https://web.archive.org/web/20090126164755/http://vcodex.com/files/h264_vlc.pdf)를 참고했다.


# CABAC (Context-adatpvie binary arithmetic coding)
H.264/MPEG-4 AVC의 특정 프로파일 및 HEVC에서 사용되는 표준으로 엔트로피 인코딩의 한 종류 이다. 가역 압축 기술의 하나로 CAVLC보다 높은 압축 효율을 보여주지만 parallelize와 vectorize가 어렵다.

## Algorithm
CABAC은 arithmetic coding을 기반으로 video encoding 표준의 필요에 의해 몇가지 변화와 발전이 있다.

- binary symbol을 encoding 함으로써 complexity 낮게 유지하고 모든 기호에서 자주 사용되는 비트에 대한 확률 모델링을 허용

- 확률 모델은 로컬 컨텍스트를 기반으로 adaptive하게 선택되며 local correlation이 있기 때문에 더 나은 률 모델링이 가능

- 양자화 된 확률 범위 및 확률 상태를 사용하여 곱셈없는 범위 분할을 사용 

![CABAC](/assets/img/postImages/CABAC.png)

