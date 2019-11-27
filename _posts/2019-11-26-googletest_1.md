---
layout: post
title:  "googletest_1"
date:   2019-11-26
comments: true
categories: cpp
---
# Googletest Primer

## Introduction: Why googletest?

1. 테스트는 독립적이고 반복 가능해야한다. 다른 테스트의 결과에 따라 그 값이 달라지는 테스트는 디버깅하기 곤란하다. => googletest는 test를 다른 object에서 돌아가도록 함으로써 test를 격리한다. googletest를 이용하면 실패한 test를 독립적으로 돌려서 빠른 디버깅이 가능하다.

2. test는 잘 구성 되어야하며 테스트하려는 코드의 구조를 반영해야한다. => googletest는 연관 테스트를 데이터와 서브 루틴 공유가 가능한 묶음으로 그룹화한다. 이 패턴은 알아보기 쉽고 test를 유지하기에 용이하다. 특히 프로젝트를 전환하면서 새로운 code를 기반으로 시작할 때 이런 일관성이 큰 도움이 된다.

3. 테스트는 이식 가능하고 재사용 가능해야 한다. google에는 플랫폼 독립적인 코드가 많이 있으며 이들의 테스트 또한 플랫폼 독립적이어야 한다. googletest는 예외가 있거나 없는 다른 컴파일러와 함께 다른 OS에서 작동하므로 googletest 테스트는 다양한 구성에서 작동 할 수 있습니다.

4. 테스트가 실패하면 이와 관련된 가능한 한 많은 정보를 제공해야한다. googletest는 첫번째 test가 실패하더라도 non-fatal failure에 대해 로그를 남기고 다음 테스트로 넘어간다. 따라서 한번 의 실행으로 여러 버그들을 찾고 고칠수 있다.

5.  테스트 프레임워크는 테스트 작성자가 오직 테스트 내용에만 집중할 수 있게 두어야한다. googletest는 사용자가 정의한 모든 테스트를 추적하여 사용자가 실행 순서를 열거할 필요 없이 알아서 실행시킨다.

6. 테스트는 빨라야 한다. googletest를 쓰면 test를 할 때마다 shared resource를 재사용할 수 있다. 자원과 관련된 부분은 오직 set-up/tear-down과정 뿐이다.

구글 테스트는 xUnit architecture에 기반했기 때문에 JUnit PyUnit 등등을 사용해 봤다면 금방 쓸 수 있다.

## Basic Concepts

test program은 하나 이상의 test suite로 이루어져 있고 test suite 또한 하나 이상의 test로 이루어져 있다. test는 assertion을 이용해서 test code의 동작 결과를 확인한다. assertion은 조건문이 true인지 확인하는데, assertion의 결과는 success, fatal failure, non-fatal failure로 구분된다. fatal failure의 경우 현재 function을 중단하며 이외의 경우 프로그램이 정상작동한다. 

만약 test suite의 여러 테스트가 object나 subroutine을 공유해야 한다면 이들을 test fixture class에 넣자

## Assertion

assertion은 function에 서로 다른 영향을 주는 두개의 버전으로 제공이 된다. ASSERT_\* 버전은 fail하면 fatal failure를 생성하며 현재 function을 중단한다. EXPECT_\* 버전은 non-fatal failure를 생성하며 현재 function을 중단하지 않는다. 보통 EXPECT_\*가 더 많은 수의 test를 진행할 수 있기때문에 선호된다. 다만 더 이상의 진행이 의미가 없다고 판단 될 경우 ASSERT_\*를 사용하자. failure에 대해서 로그를 남기고 싶다면 << 를 사용하자.
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

한편 Basic Assertion을 이용하는 것보다는 각 타입에 맞는 assertion을 사용하자. ASSERT_TRUE(a==b) 보다는 ASSERT_EQ(a,b) 에서 실패했을 시 a!=b임을 알고 이를 출력 해주기 때문이다. ASSER_TRUE 는 어떤 condition을 사용하는지에 대해서는 상세히 알지 못한다.

### Binary Comparison

Fatal assertion          | Nonfatal assertion       | Verifies
------------------------ | ------------------------ | --------------
`ASSERT_EQ(val1, val2);` | `EXPECT_EQ(val1, val2);` | `val1 == val2`
`ASSERT_NE(val1, val2);` | `EXPECT_NE(val1, val2);` | `val1 != val2`
`ASSERT_LT(val1, val2);` | `EXPECT_LT(val1, val2);` | `val1 < val2`
`ASSERT_LE(val1, val2);` | `EXPECT_LE(val1, val2);` | `val1 <= val2`
`ASSERT_GT(val1, val2);` | `EXPECT_GT(val1, val2);` | `val1 > val2`
`ASSERT_GE(val1, val2);` | `EXPECT_GE(val1, val2);` | `val1 >= val2`

Argument의 경우에는 딱 한번만 실행되기 때문에 argument 가 side effect를 가지더라도 문제 없다. 다만 일반적으로 C/C++ 함수 특징상 argument의 연산 순서를 명확히 알 수 없기 때문에 이를 유의해서 작성해야 한다. 한편 ASSERT_EQ()는 pointer에 대해서는 pointer의 equality를 따지기 때문에 C string을 사용한다면 string의 비교가 아닌 string의 memory location에 대해서 비교를 하게 된다.  따라서 ASSERT_STREQ() 의 사용이 권장된다. 다만 pointer에 대한 비교이 이므로 두개의 string object를 비교할 때는 ASSERT_EQ 사용하자. 한편 pointer 비교를 할 때에는 \*\_EQ(ptr, nullptr), \*\_NE(ptr, nullptr)을 \*\_EQ(ptr, NULL), \*\_NE(ptr, NULL) 대신 사용하자. nullptr는 pointer 타입이지만 NULL은 아니기 때문이다.

### String Comparison

**이 assertion은 C string을 위해 사용된다. string object를 비교하고 싶다면 EXPECT_EQU 이나 EXPECT_NE등을 사용하자**

| Fatal assertion                | Nonfatal assertion             | Verifies                                                 |
| --------------------------     | ------------------------------ | -------------------------------------------------------- |
| `ASSERT_STREQ(str1,str2);`     | `EXPECT_STREQ(str1,str2);`     | the two C strings have the same content            |
| `ASSERT_STRNE(str1,str2);`     | `EXPECT_STRNE(str1,str2);`     | the two C strings have different contents          |
| `ASSERT_STRCASEEQ(str1,str2);` | `EXPECT_STRCASEEQ(str1,str2);` | the two C strings have the same content, ignoring case   |
| `ASSERT_STRCASENE(str1,str2);` | `EXPECT_STRCASENE(str1,str2);` | the two C strings have different contents, ignoring case |

NULL pointer 와 빈 string은 다르게 취급된다.

## Simple Tests

이제 test를 한번 만들어 보자

1. TEST() macro를 사용하여 test function의 이름을 정의한다. return 값이 없는 일반적인 C++ 함수의 형태이다.

2. 이 함수 에서 포함하고자 하는 code를 작성하고 assertion을 이용해서 value를 check하자

3. 테스트 결과는 assertion에 의해서 결정된다.

```c++
TEST(TestSuiteName, TestName) {
  ... test body ...
}
```

TEST에서 첫번째 argument는 test suite의 이름이고 두번째 argument는 test의 이름을 의미한다. test의 전체 이름은 이 둘을 다 포함한다. 

## Test Fixtures :  같은 데이터를 여러 테스트 에서 써보자

1. ::testing::Test로부터 class 를 상속 받는다. fixture의 멤버에는 sub-class로 접근 하는걸 원하기 때문에, body는 protected: 로 시작한다.

2. class 안에는 쓰고 싶은 object를 선언한다.

3. 필요하다면 생성자나 SetUp() function을 써서 각각의 테스트마다 사용할 object를 준비해주자. SetUp을 잘못 타이핑 하는 경우가 종종 있기 때문에 항상 override를 사용해서 문제가 없도록 체크해주자.

4. 필요하다면 소멸자나 TearDown() function을 사용해서 SetUp()에서 할당한 자원을 해제해주자. 

5. 필요하다면 테스트에서 공유할 subroutine을 정의하자

fixture를 사용하려면 TEST() 대신에 TEST_F()를 사용해야 한다.

```c++
TEST_F(TestFixtureName, TestName) {
  ... test body ...
}
```

TEST() 처럼 첫번째 argument는 test suite name 이지만 TEST_F() 는 이것을 반드시 test fixture class의 이름으로 설정해야 한다. 또한 TEST_F()에서 사용하는 fixture class에 대한 정의는 반드시 TEST_F() 이전에 작성되어야 한다.

## Invoking the Tests

TEST()와 TEST_F()가 google test에 등록되기 때문에, 정의하고 그냥 RUN_ALL_TESTS()를 호출하면 test가 실행된다. 전부 성공하면 0을 return 하고 아닐 경우 1을 return 한다.

RUN_ALL_TESTS() 매크로를 호출 하면: 

*   모든 googletest flag를 저장한다.

*   첫번째 test를 수행하기 위해서 test fixture object를 만든다.

*   SetUp()을 통해 초기화.

*   fixture object를 가지고 test를 수행.

*   TearDown()으로 자원 정리.

*   fixture를 지운다.

*   모든 googletest flag를 복구한다.

*   모든 테스트가 수행 될 때까지 이를 반복한다.

> 중요: ** RUN_ALL_TESTS()의 return 값을 무시하면 안된다. **
> exit code에 dependent하게 디자인 되었으므로, main()은 반드시 RUN_ALL_TESTS의 값을 return 해야 한다.

 Also, you should call `RUN_ALL_TESTS()` only **once**. Calling it more than
 once conflicts with some advanced googletest features (e.g., thread-safe
 [death tests](advanced.md#death-tests)) and thus is not supported.

## Writing the main() Function

아마 대부분의 사용자는 main function을 사용하는 것보다는 test를 gtest_main을 link할 것이다. 이 파트는 custom 한 것을 한 뒤에 test를 돌릴 필요성이 있을 대 유용할 것이다. 다 무시하고 main 만 보자

```c++
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
```

::testing::InitGoogleTest() 함수는 command line에서 googletest flag를 받는데 이를 통해서 사용자가 test program의 행동을 컨트롤 할 수 있다. RUN_ALL_TESTS() 전에 이를 반드시 호출해야 하며 그렇지 않으면 flag의 초기화가 제대로 진행되지 않는다. 


## Known Limitations
googletest는 thread-safe로 디자인 되어 있다. 다만 이는 pthreads 라이브러리에서는 그런 것이며 지금(2019/11/27)은 아직 다른 시스템에 대해서는 unsafe하다. 물론 테스트가 보통 main thread에서 일어나기에 문제는 없다.
