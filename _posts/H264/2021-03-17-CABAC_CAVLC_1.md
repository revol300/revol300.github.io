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
H.264에서는 코덱 사용목적에 따라 프로파일을 나누고, 어떤 프로파일을 사용하는지에 따라 다른 방법을 통해 encoding 및 decoding을 지원한다. 특히 그중에 차이를 보이는 점이 메크로블록의 DCT coefficient를 압축하는 과정이다. 아래의 table에 적힌 대부분의 parameter의 경우에는 Exponential Golomb이라 불리는 기본적인 부화하 방법이 사용된다. 이중 노ㅠ은 부호화 효율이 요구되는 DCT 계수 (Residual data)에 대해서는 프로파일의 종류에 따라 CAVLC(Context-adaptive variable-length coding) 혹은 CABAC(Context-adaptive binary arithmetic coding)을 이용한다. 간단히 장단을 비교하자면 CAVLC는 압축효율이 좀 떨어지지만 연산량이 적고 CABAC의 경우에는 연산량이 많지만 압축효율이 좋다. 그러한 연유로 자연스레 컴퓨터의 성능이 좋아지면서 H.265에서는 둘중에 하나를 선택하는 것이 아닌 CABAC을 사용한다고 한다. 이제 CAVLC 및 CABAC가 어떤 방식으로 압축을 진행하는지에 대해서 알아보자

| Parameters                                          | Description                                                           |
|-----------------------------------------------------|-----------------------------------------------------------------------|
| Sequence-, picture- and slice-layer syntax elements |                                                                       |
| Macroblock type mb_type                             | Prediction method for each coded macroblock                           |
| Coded block pattern                                 | Indicates which blocks within a macroblock contain coded coefficients |
| Quantizer parameter                                 | Transmitted as a delta value for previous value of QP                 |
| Reference frame index                               | Identify reference frame(s) for inter prediction                      |
| Motion vector                                       | Transmitted as a difference (mvd) from predicted motion vector        |
| Residual data                                       | Coefficient data for each 4x4 or 2x2 block                            |

# Exp-Golomb entropy coding
Exp-Golomb은 다음과 같은 표현식으로 요약할 수 있다.
[M zeros]1[INFO]

예를 들어 우리가 음이 아닌 정수를 표현한다고 하자. INFO에 들어갈 수 있는 0,1의 배열을 작은 순서부터 쓰면 다음과 같다.

''

'0'

'1'

'00'

'01'

'10'

'11'

'000'

...

이를 음이 아닌 정수 0,1,2,3,4 ... 에 대응하면

0 : ''

1 : '0'

2 : '1'

3 : '00'

4 : '01'

5 : '10'

6 : '11'

7 : '000'

...

이를 그대로 Exp-Golomb의 표현식에 대입하면

0 : 1

1 : 010

2 : 011

3 : 00100

4 : 00101

5 : 00110

6 : 00111

7 : 0001000

...

한편 부호가 있는 정수에 대해서는 다음과 같이 표현된다

0 : 0

1 : 010

-1 : 011

2 : 011

-2 :00100

...

#CAVLC (Context-adaptive variable-length coding)
다음 [링크](https://web.archive.org/web/20090126164755/http://vcodex.com/files/h264_vlc.pdf)를 참고했다.
CAVLC는 entropy_coding_mode 가 0으로 설정되어 있을 때 DCT 계수를 부호화하는데 사용된다.
DCT 계수에는 다음과 같은 특징이 있다.

- 계수가 0이 많고, 그밖에도 1, -1 이 대부분이다
- 0이 아닌 계수는 주변 블록과 상관관계가 있다.
- 저주파수 성분이 0이 아닌 경우가 많고 고주파수로 갈수록 0이 많아진다.

이 같은 특징을 바탕으로 CAVLC가 구성되었으며 다음과 같은 과정으로 DCT 계수가 부호화 된다.

## coeff_token : 0이 아닌 계수의 총 수 (TotalCoeffs) 와 TrailingOnes(연속하는 절대값 1인 계수의 개수)를 인코딩한다. 
이 과정에서 TotalCoeffs는 4x4기준으로 0에서 16사이의 값을 가진다. TrailingOnes(T1)은  0에서 3까지의 값이 가능하며 연속하는 +/-1 의 개수가 3보다 많다면 마지막 3개만을 TrailingOnes로 취급하고 나머지는 다른 계수와 동일하게 취급한다. TotalCoeffs와 TrailingOnes는 추후 VLC table에 사용된다. VLC table은 Num-VLC0, Num-VLC1, Num-VLC2 그리고 Num-FLC가 있다. 이중 어떤 VLC table을 사용하는지는 인접한 매크로 블록의 TotalCoeffs에 따라 달라진다. 일반적으로 좌측 블록의 TotalCoeffs \[Left_Total_coeffs_num\]와 상단 블록의 TotalCoeffs \[Upper_Total_coeffs_num\]의 평균값\[N = Round((Left_total_coeffs_num + Upper_total_coeffs_num)/2)\]에 따라 결정된다. 만약 좌측 블록이나 상단 블록이 없다면 Num-VLC0를 사용한다.

| N           | Table for coeff token |
|-------------|-----------------------|
| 0,1         | Num-VLC0              |
| 2,3         | Num-VLC1              |
| 4,5,6,7     | Num-VLC2              |
| 8 or above  | FLC                   |

## TrailingOnes 값에 대한 부호 값을 저장
최대 3개의 TrailingOnes에 대해 부호 값을 저장한다

# CABAC (Context-adatpvie binary arithmetic coding)
H.264/MPEG-4 AVC의 특정 프로파일 및 HEVC에서 사용되는 표준으로 엔트로피 인코딩의 한 종류 이다. 가역 압축 기술의 하나로 CAVLC보다 높은 압축 효율을 보여주지만 parallelize와 vectorize가 어렵다.

## Algorithm
CABAC은 arithmetic coding을 기반으로 video encoding 표준의 필요에 의해 몇가지 변화와 발전이 있다.

- binary symbol을 encoding 함으로써 complexity 낮게 유지하고 모든 기호에서 자주 사용되는 비트에 대한 확률 모델링을 허용

- 확률 모델은 로컬 컨텍스트를 기반으로 adaptive하게 선택되며 local correlation이 있기 때문에 더 나은 률 모델링이 가능

- 양자화 된 확률 범위 및 확률 상태를 사용하여 곱셈없는 범위 분할을 사용 

![CABAC](/assets/img/postImages/CABAC.png)

