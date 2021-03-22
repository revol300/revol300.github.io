---
layout: post
title:  "CAVLC vs CABAC (1)"
date:   2021-03-17
comments: true
excerpt: CAVLC
categories: multimedia
tag:
- multimedia 
---

# Introduction

H.264에서는 코덱 사용목적에 따라 프로파일을 나누고, 어떤 프로파일을 사용하는지에 따라 다른 방법을 통해 encoding 및 decoding을 지원한다. 특히 그중에 차이를 보이는 점이 아래 Table에 쓰여진 각 Parameter를 압축하는 과정이다. 프로파일에 따라 DCT coefficient에 대해서는 CAVLC(Context-adaptive variable-length coding)를 사용하고 나머지 parameter에 대해서는 Exponential Golomb 을 사용하는 Encoding 방법과 CABAC(Context-adaptive binary arithmetic coding)만을 이용하여 Encoding 하는 방법이 사용된다. 간단히 장단을 비교하자면 CAVLC는 압축효율이 좀 떨어지지만 연산량이 적고 CABAC의 경우에는 연산량이 많지만 압축효율이 좋다. 그러한 연유로 자연스레 컴퓨터의 성능이 좋아지면서 H.265에서는 둘중에 하나를 선택하는 것이 아닌 CABAC을 사용한다고 한다. 이번 글에서는 DCT 계수를 제외한 나머지 element에 대해서 주로 사용되는 Exp-Golomb과 비교적 적은 연산을 요하는 CAVLC에 대해서 써보고자한다.

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

# CAVLC (Context-adaptive variable-length coding)

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

## 남은 0이 아닌 coefficient 값에 대해 encoding 을 수행

이제 남은 coefficient 값에 대해서 encodig 을 수행한다. encoding 은 Level_VLC0 ~ Level_VLC6의 7개의 테이블을 참조하여 이루어지며 끝에 있는 coefficient부터 다음과 같은 방식으로 이루어진다.
Level_VLC 테이블은 다음 [링크](https://patentimages.storage.googleapis.com/8a/41/24/180e26a06896c6/KR20070069381A.pdf)의 도면3을 참고하면 된다

- Level_VLC0 테이블을 참고해서 인코딩을 시작한다 (만약 coefficient의 개수가 10개 이상이고 TrailingOnes값이 3 미만이라면 Level_VLC1을 참고한다.)
- 계수의 크기가 테이블의 threshold다 크면 다음 VLC 테이블을 선택하여 인코딩한다. 테이블 Level이 올라간만큼 suffix값을 표시한다.

VLC 테이블의 threshold

| Current VLC Table | Threshold |
|-------------------|-----------|
| VLC0              | 0         |
| VLC1              | 3         |
| VLC2              | 6         |
| VLC3              | 12        |
| VLC4              | 24        |
| VLC5              | 48        |
| VLC6              | N/A       |

## 마지막 coefficient 전의 0의 개수를 부호화
처음부터 마지막 coefficient까지 0의 개수를 TotalZeros로 저장한다 (VLC를 사용). 이후 끝에서 부터 해당 coefficient 앞에 있어야할 0의 개수를 저장한다.

# CAVLC example
![CAVLC_example](/assets/img/postImages/CAVLC_example.png)

## Encoding
먼저 위 값을 직렬화하면 다음과 같이 쓸 수 있다.

0, 3, 0, 1, -1, -1, 0, 1, 0 ...

### coeff_token : 0이 아닌 계수의 총 수 (TotalCoeffs) 와 TrailingOnes(연속하는 절대값 1인 계수의 개수)를 인코딩한다. 

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |

왼쪽이나 위에 블록이 없다고 가정하면 여기서 사용되는 table은 Num-VLC0가 된다. 전체  coefficient의 개수가 5개 TrailingOnes는 4이지만 3개까지만 가능하므로 TotalCoeffs=5, TrailingOnew=3이 된다. 이에 해당하는 값을 Num-VLC0에서 찾으면 000100이 된다.

### TrailingOnes 값에 대한 부호 값을 저장

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |
| sign          | +                             | 0                                  |
| sign          | -                             | 1                                  |
| sign          | -                             | 1                                  |

1,-1,-1,1 중에 마지막 3개만 취하므로 -1,-1,1 만 사용하고 Encoding은 끝에서부터 이뤄지기 때문에 1, -1, -1 순이 되며 +, -, -이므로 숫자로는 0, 1, 1로 저장된다

### 남은 0이 아닌 coefficient 값에 대해 encoding 을 수행

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |
| sign          | +                             | 0                                  |
| sign          | -                             | 1                                  |
| sign          | -                             | 1                                  |
| Coefficient   | +1 (use Level_VLC0)           | 1(prefix)                          |
| Coefficient   | +3 (use Level_VLC1)           | 001(prefix) 0(suffix)              |

이제 남은 coefficient 인 3, 1을 Ecoding한다. 1은 VLC0를 사용하지만 3은 threshold를 넘으므로  VLC1을 사용한다

### 마지막 coefficient 전의 0의 개수를 Encoding

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |
| sign          | +                             | 0                                  |
| sign          | -                             | 1                                  |
| sign          | -                             | 1                                  |
| Coefficient   | +1 (use Level_VLC0)           | 1(prefix)                          |
| Coefficient   | +3 (use Level_VLC1)           | 001(prefix) 0(suffix)              |
| TotalZeros    | 3                             | 111                                |
| run_before(4) | ZerosLeft=3; run_before=1     | 10                                 |
| run_before(3) | ZerosLeft=2; run_before=0     | 1                                  |
| run_before(2) | ZerosLeft=2; run_before=0     | 1                                  |
| run_before(1) | ZerosLeft=2; run_before=1     | 01                                 |
| run_before(0) | ZerosLeft=1; run_before=1     | No code required; last coefficient |

coefficient가 있는 1까지 쓰면 0, 3, 0, 1, -1, -1, 0, 1. 여기서 0의 총 개수는 3이다. 0을 빼고 다시 쓰면 3, 1, -1, -1, 1이고 각 coefficient 앞의 0의 개수는 3: 1개, 1: 1개, -1: 0개, -1: 0개, 1: 1개로, 11001이되며 끝에서부터 Encoding되고 맨 앞의 값이 없어도 TotalZeros로 알수 있으므로 1개, 0개, 0개, 1개를 Code화

## Decoding

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=, TrailingOnes=   | 0000100                            |
| sign          |                               | 0                                  |
| sign          |                               | 1                                  |
| sign          |                               | 1                                  |
| Coefficient   |                               | 1(prefix)                          |
| Coefficient   |                               | 001(prefix) 0(suffix)              |
| TotalZeros    |                               | 111                                |
| run_before(4) |                               | 10                                 |
| run_before(3) |                               | 1                                  |
| run_before(2) |                               | 1                                  |
| run_before(1) |                               | 01                                 |

이제 Decoding을 수행해보자

### TotalCoeffs, TrailingOnes

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |

주어진 Code를 Num-VLC0에 조회해보면 TotalCoeffs=5, TrailingOnes=3이다. 그러면 해당 결과에 따라 아래와 같이 1이 존재함을 할 수 있다.

```
1, 1, 1
```

### TrailingOnes의 부호값 설정

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |
| sign          | +                             | 0                                  |
| sign          | -                             | 1                                  |
| sign          | -                             | 1                                  |

sign 값을 이용하여 부호를 알아낼 수 있다.

```
-1,-1,1
```

### 나머지 Coefficient 추가

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |
| sign          | +                             | 0                                  |
| sign          | -                             | 1                                  |
| sign          | -                             | 1                                  |
| Coefficient   | +1 (use Level_VLC0)           | 1(prefix)                          |
| Coefficient   | +3 (use Level_VLC1)           | 001(prefix) 0(suffix)              |

나머지 코드를 통해 남은 Coefficient값을 적어준다.

```
+3,+1,-1,-1,1
```

### 0을 추가

| Element       | Value                         | Code                               |
|---------------|-------------------------------|------------------------------------|
| coeff_Token   | TotalCoeffs=5, TrailingOnes=3 | 0000100                            |
| sign          | +                             | 0                                  |
| sign          | -                             | 1                                  |
| sign          | -                             | 1                                  |
| Coefficient   | +1 (use Level_VLC0)           | 1(prefix)                          |
| Coefficient   | +3 (use Level_VLC1)           | 001(prefix) 0(suffix)              |
| TotalZeros    | 3                             | 111                                |
| run_before(4) | ZerosLeft=3; run_before=1     | 10                                 |
| run_before(3) | ZerosLeft=2; run_before=0     | 1                                  |
| run_before(2) | ZerosLeft=2; run_before=0     | 1                                  |
| run_before(1) | ZerosLeft=2; run_before=1     | 01                                 |
| run_before(0) | ZerosLeft=1; run_before=1     | No code required; last coefficient |

TotalZeros 및 run_before 값을 이용해서 0을 추가해준다.

```
0, 3, 1, -1, -1, 0, 1
```

이제 끝에 0으로 채워 16자리를 맞춰준다.

```
0, 3, 1, -1, -1, 0, 1, 0 ...
```

# Conclusion

이번 글에서는 H.264에서 DCT Coefficient를 Encoding 하는데 사용되는 CABAC 및 CAVLC중 비교적 적은 연산을 요구하는 CAVLC에 대해 적어보았다. 다음 글에서는 CABAC에 대해서 간단히 써보려고 한다.
