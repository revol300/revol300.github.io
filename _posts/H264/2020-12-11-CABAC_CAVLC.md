---
layout: post
title:  "CABAC vs CAVLC"
date:   2020-12-11
comments: true
excerpt: CABAC vs CAVLC 
categories: multimedia
tag:
- multimedia 
---

# CABAC (Context-adatpvie binary arithmetic coding)
H.264/MPEG-4 AVC의 특정 프로파일 및 HEVC에서 사용되는 표준으로 엔트로피 인코딩의 한 종류 이다. 가역 압축 기술의 하나로 CAVLC보다 높은 압축 효율을 보여주지만 parallelize와 vectorize가 어렵다.

## Algorithm
CABAC은 arithmetic coding을 기반으로 video encoding 표준의 필요에 의해 몇가지 변화와 발전이 있다.
- binary symbol을 encoding 함으로써 complexity 낮게 유지하고 모든 기호에서 자주 사용되는 비트에 대한 확률 모델링을 허용

- 확률 모델은 로컬 컨텍스트를 기반으로 adaptive하게 선택되며 local correlation이 있기 때문에 더 나은 률 모델링이 가능

- 양자화 된 확률 범위 및 확률 상태를 사용하여 곱셈없는 범위 분할을 사용 

![CABAC](/assets/img/postImages/CABAC.png)


