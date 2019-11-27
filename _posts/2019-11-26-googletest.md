---
layout: post
title:  "googletest"
comments: true
date:   2019-11-26 
categories: cpp 
---

About googletest

1. 테스트는 독립적이고 반복 가능해야한다. 다른 테스트의 결과에 따라 그 값이 달라지는 테스트는 디버깅하기 곤란하다. => googletest는 test를 다른 object에서 돌아가도록 함으로써 test를 격리한다. googletest를 이용하면 실패한 test를 독립적으로 돌려서 빠른 디버깅이 가능하다.

2. test는 잘 구성되어야하며 테스트하려는 코드의 구조를 반영해야한다. => googletest는 연관 테스트를 데이터와 서브 루틴 공유가 가능한 묶음으로 그룹화한다. 이 패턴은 알아보기 쉽고 test를 유지하기에 용이하다. 특히 프로젝트를 전환하면서 새로운 code를 기반으로 시작할때 이런 일관성이 큰 도움이 된다.

3. 테스트는 이식 가능하고 재사용 가능해아한다. google에는 플랫폼 독립적인 코드가 많이 있으며 이들의 테스트 또한 플랫폼 독립적이어야 한다. googletest는 예외가 있거나 없는 다른 컴파일러와 함께 다른 OS에서 작동하므로 googletest 테스트는 다양한 구성에서 작동 할 수 있습니다.

4. 테스트가 실패하면 이와 관련된 가능한한 많은 정보를 제공해야한다.  googletest는 첫번째 test가 실패하더라도 다음 테스트로 넘어간다. non-fatal failure에 대해 report를 받고 . 따라서 한번 의 실행으로 여러 버그들을 찾고 고칠수 있다.

5.  테스트 프레임워크는 테스트 작성자가 오직 테스트 내용에만 집중할 수있게 도와야한다. googletest는 사용자가 정의한 모든 테스트를 추적하여 사용자가 실행순서를 열거할 필요 없이 알아서 실행시킨다.

6. 테스트는 빨라야 한다. googletest를 쓰면 test를 할때마다 shared resource를 재사용할 수 있다. 자원과 관련된 부분은 오직 set-up/tear-down과정 뿐이다.

구글 테스트는 xUnit architecture에 기반했기 때문에 JUnit PyUnit 등등을 사용해 봣다면 금방 쓸 수 있다.

구조

test는 assertion을 이용해서 test code의 동작 결과를 확인한다. assertion은 조건문이 treu인지 확인하는데, assertion의 결과는 success, fatal failure, non-fatal failure로 구분된다. fatal failure의 경우 현재 function을 중단하며 이외의 경우 프로그램이 정상작동한다. 

테스트 프로그램의 구조는 아래와 같다.

test program +--------------------> test suite +----------------> test
             |                                 |
             |                                 +----------------> test
             |                                 |
             |                                 +----------------> test
             |
             |
             +--------------------> test suite +----------------> test

*만약 test suite의 여러 테스트가 object나 subroutine을 공유해아 한다면 이들을 test fixture class에 넣자

Assertion
assertion은 function에 서로 다른 영향을 주는 두개의 버전으로 제공이 된다. ASSERT_* 버전은 fail하면 fatal failure를 생성하며 현재 function을 중단한다. EXPECT_* 버전은 non-fatal failure를 생성하며 현재 function을 중단하지 않는다. 보통 EXPECT_*가 더많은 수의 test를 진행할 수 있기때문에 선호된다. 다만 더이상의 진행이 의미가 없다고 판단 될 경우 ASSERT_*를 사용하자. failure에 대해서 로그를 남기고 싶다면 << 를 사용하자.
ex.)

```c++
ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

for (int i = 0; i < x.size(); ++i) {
  EXPECT_EQ(x[i], y[i]) << "Vectors x and y differ at index " << i;
}
```

### Basic Assertions

Fatal assertion            | Nonfatal assertion         | Verifies
-------------------------- | -------------------------- | --------------------
`ASSERT_TRUE(condition);`  | `EXPECT_TRUE(condition);`  | `condition` is true
`ASSERT_FALSE(condition);` | `EXPECT_FALSE(condition);` | `condition` is false

한편 Basic Assertion을 이용하는 것보다는 각 타입에 맞는 assertion을 사용하자. ASSERT_TRUE(a==b) 보다는 ASSERT_EQ(a,b) 에서 실패했을 시 a!=b임을 알고 이를 출력해주기 때문이다. ASSER_TRUE 는 어떤 condition을 사용하는지에 대해서는 상세히 알지 못한다.

### Binary Comparison

Fatal assertion          | Nonfatal assertion       | Verifies
------------------------ | ------------------------ | --------------
`ASSERT_EQ(val1, val2);` | `EXPECT_EQ(val1, val2);` | `val1 == val2`
`ASSERT_NE(val1, val2);` | `EXPECT_NE(val1, val2);` | `val1 != val2`
`ASSERT_LT(val1, val2);` | `EXPECT_LT(val1, val2);` | `val1 < val2`
`ASSERT_LE(val1, val2);` | `EXPECT_LE(val1, val2);` | `val1 <= val2`
`ASSERT_GT(val1, val2);` | `EXPECT_GT(val1, val2);` | `val1 > val2`
`ASSERT_GE(val1, val2);` | `EXPECT_GE(val1, val2);` | `val1 >= val2`
