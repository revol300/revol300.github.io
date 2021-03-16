// 템플릿 메타프로그래밍

// 컴파일 시점에 유형 조작

template <typename T>
using contained_type_t = decltype(*begin(T()));

// 위 표현식의 문제점 T는 기본 생성 가능하다는 점에 의존한다. 
// 이 문제를 해결하려면 생성자 호출을 std::declval<T>() 유틸리티 메타함수로 대체해야 한다.

// contained_type_t를 이용하면 컬렉션 내의 항목에 대한 타입을 유추할 수 있다
template <typename C,
          typename R = contained_type_t<C>>
R sum(const C& collection) {
  ...
}

// 추뢴된 유형 디버깅
// contained_type_t의 두 번째 문제점은 원하는 것을 수행하지 않는다는 점이다.
// 즉 특정 유형을 예상했지만 컴파일러는 다른 유형을 갖게하는 경우가 생기며 이를 방지하기 위해 유형검사를 해야한다.
// contained_type_t.에 의해 추론된 유형 검사

template <typename T>
class error;

error<contained_type_t<std::vector<std::string>>>();

// 위 코드는 다음과 유사한 컴파일 오류를 발생시킨다.

error: invalid use of incomplete type

'class error<const std::string&>'

// std::string이 아닌 const std::string&으로 추론

// contained_type_t 메타함수의 완전한 구현
template <typename T>
using contained_type_t =
  std::remove_cv_t<
    std::remove_reference_t<
      decltype(*begin(std::declval<T>()))
    >
  >;

// 이제 contained_type_t<std::vector<std::string>>의 결과는 std::string이 된다.

// static_assesrt도 디버깅에 유용하다.

static_assert(
  std::is_same<int, contained_type_t<std::vector<int>>>(),
  "std::vector<int> should contain integers");
static_assert(
  std::is_same<int, contained_type_t<std::vector<std::string>>>(),
  "std::vector<std::string> should contain strings")
static_assert(
  std::is_same<int, contained_type_t<std::vector<person_t*>>>(),
  "std::vector<person_t> should contain people")

// 컴파일 동안의 패턴 일치
// std::is_same의 가능한 구현
template<typename T1, typename T2>
struct is_same : std::false_type{};

template<typenaem T>
struct is_same<T,T> : std::true_type{};

// remove_reference_t 메타함수의 구현
template <typename T>
// 일반적인 경우 remove_reference<T>::type은 유형 T다.
// 즉, 받은 유형과 동일한 유형을 반환한다.
struct remove_reference {
  using type = T;
};

template <typename T>
// lvalue 참조 T&을 받으면 참조를 제거하고 T를 반환한다.
struct remove_reference<T&> {
  using type = T;
};

template <typename T>
// rvalue 참조 T&&을 받으면 참조를 제거하고 T를 반환한다.
struct remove_reference<T&&> {
  using type = T;
}

// 좀더 편리한 버전
template <typename T>
using remove_reference_t<T> =
  typename remove_reference<T>::type;

// 위에서 사용한 템플릿을 사용해보자
template <typename C,
          typename R = contained_type_t<C>>
R sum_iterable(const C& collection) {
  return std::accumulate(begin(collection), end(collection), R());
}

// 우형에 관한 메타정보 제공
// 있는거 쓰자 그냥... value_type이라는 걸 준다는디..
// expected보면 이런게 있다 
template <typename T, typename E>
class expected {
public:
  using value_type = T;
  ...
};

template <typename C,
          typename R = typename C::value_type>
R sum_collection(const C& collection) {
  return std::accumulate(begin(collection), end(collection), R());
}

// 컴파일 시점에 유형 속성 검사

