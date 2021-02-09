//모나드

// FP에서 함수자는 호출 연산자를 가진 클래스가 아니다
// class F는 transform (또는 map) 함수가 정의돼 있으면 함수자다
// transform 함수는 다음의 2가지 규칙을 준수해야한다
// identify 함수로 함수자 인스턴스를 반환하면 동일한 함수자 인스턴스를 반환한다
// f | transform([](auto value) { return value; }) == f
// 하나의 함수로 함수자를 변환한 다음 또 다른 함수로 변환하는 것은 이들 두 함수의 조합으로 함수자를 변환하는 것과 같다
// f | transform(t1) | transform(t2) ==
// f | transform([=](auto value) {return t2(t1(value)); })
// std::transform 및 view::transform 즉 stl 과 범위의 일반 컬렉션이 함수자에 포함된다.

// 옵션 값 처리
// std::optional에 대한 변환 함수 정의하기
template <typename T1, typename F>
auto transform(const std::optional<T1>& opt, F f)
  -> decltype(std::make_optional(f(opt.value()))) { // 값이 없을 때 {}만 반환하므로 반환 유형을 지정하자
  if (opt) { 
    return std::make_optional(f(opt.value()));
  } else {
    return {}; 
  }
}
// 대안으로 range를 사용하여 값이 있으면 한 요소의 범위를 제공하는 범위 뷰를 만들고
// 값이 없으면 빈 범위를 만들 수 있다 => 파이프 사용 가능

// 이제 만약 사용자의 로그인 했으면 값을 가지는 변수를 정의해보자
std::optional<std::string> current_login;

// 이 값을 이용하여 full_name을 구하고 이를 html로 표시해주는 두개의 함수를 정의해보자

std::string user_full_name(const std::string& login);
std::string to_html(const std::string& text);

// 이를 위에서 정의한 transform을 이용하여 html을 만들어주는 연산을 하면
// 다음과 같이 표시된다.
transform(
    transform(
      current_login,
      user_full_name),
    to_html);

// 하지만 이를 optional 대신 range를 사용할 수 있다
auto login_as_range = as_range(current_login);

// 그러면 transform을 바로 사용하여 다음과 같이 앞선 연산을 나타낼 수 있다.
login_as_range | view::transform(user_full_name)
               | view::transform(to_html); 

// 큰 차이가 없다! => std::optional이든지 아니던지 관심이 없음
// 물론 엄밀한 의미에서 std::optional을 range로 바꾸고 다시 이를 되돌리는 과정

// 모나드: 함수자에게 더 많은 능력을
// 앞선 예시에서
transform(current_login, user_full_name);
// 의 반환값은 std::optional<std::string>이 아닌 std::optional<std::optional<std::string>>을 반환한다
// 즉 수행하는 변환이 많을 수록 더 많은 중첩이 발생한다
// 모나드가 여기서 쓰인다 모나드 M<T>는 자신에 대해 정의된 추가적인 함수(한 단계 수준의 중첩을 제거하는 함수)를 갖는 함수자다.
join: M<M<T>> -> M<T>

//이를 이용하면 다음과 같은 코드 작성으로 중첩을 제거할 수 있다

join(transform(
      join(transform(
          current_login,
          user_full_name)),
      to_html));

// 범위 표기법을 이용한다면
auto login_as_range = as_range(current_login);
login_as_range | view::transform(user_full_name)
               | view::join
               | view::transform(to_html)
               | view::join;

// join을 없애자
construct : T -> M<T>
mbind     : (M<T1>, T1 -> M<T2>) -> M<T2>

// 규칙
// f: T1 -> M<T2>와 유형 T1의 값 a가 있다면 이 값을 모나드 M에 래핑하고 함수 f로 이를 바인드하는 것은 이에 대해 함수 f를 호출하는 것과 같다
mbind(construct(a), f) == f(a)

// 래핑된 값을 생성 함수에 바인딩하면 래핑된 동일한 값을 얻는다.
mbind(m, construct) == m

// mbind 연산의 결합성
mbind(mbind(m,f), g) == mbind(m ,[] (auto x) {
    return mbind(f(x), g)
    })

// 기본 예제
// std::vector로 시작

template <typename T, typename F>
auto transform(const std::vector<T>& xs, F f) {
  return xs | view::transform(f) | to_vector;
}

template <typename T>
std::vector<T> make_vector(T&& value) {
  return {std::forward<T>(value)};
}


