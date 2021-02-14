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

// 모나드 M
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
// 함수자는 하나의 템플릿 매개변수를 갖는 클래스 탬플릿이다
// 하나의 벡터를 가질 수 있는 transform 함수와 그 요소를 변환할 수 있는 함수가 필요하다.
// transform은 변환된 요소의 벡터를 반환한다.

template <typename T, typename F>
auto transform(const std::vector<T>& xs, F f) {
  return xs | view::transform(f) | to_vector;
}
// 이제 vector는 transform을 갖는 함수자이다

// 이제 모나드에 필요한 constructor를 만들자
template <typename T>
std::vector<T> make_vector(T&& value) {
  return {std::forward<T>(value)};
}

// 마지막으로 필요한 mbind 함수를 구현하자
template <typename T, typename F>
auto mbind(const std::vector<T>& xs, F f) { // f는 유형 T의 값을 받아 T 또는 다른 유형의 벡터를 반환한다.
  auto transformed =  //f를 호출하고 벡터 범위를 만든다. 이 범위를 벡터의 벡터로 변환한다.
    xs | view::transform(f)
       | to_vector;
  return transformed // 벡터에 대한 벡터를 원하지 않고 단일 벡터 내의 모든 값을 원한다.
       | view::join
       | to_vector;
}

// std::vector 는 모나드이다.

// 범위와 모나드의 내포
// mbind가 필요한가? => mbind를 사용하면 원본 컬렉션의 각 요소에 대해 하나의 새 요소를 생성할 수 있을 뿐만 아니라
// 원하는 만큼의 요소를 생성할 수 있다

// ex.) mbind 관점에서의 필터링
template <typename C, typename P>
auto filter(const C& collection, P predicate) {
  return collection
    | mbind([=](auto element) {
        return view:::single(element)
          | view::take(predicate(element)? 1: 0);
        });
}

// 피타고라스 삼원수 생성하기
views::ints(1)
  | mbind([](int z) {
      return view::ints(1,z)
      | mbind([z](int y) {
          return view::ints(y,z)
          | view::transform([y,z](int x) {
              return std::make_tuple(x,y,z);
              });
          });
      })
  | filter([](auto triple) {
      ... // 이제 삼원수의 목록이 생겼고 피타고라스 삼원수가 아닌 정수는 필터링해야 한다.
      });

// 범위 내포를 이용한 피타고라스 삼원수 생성하기
// for_each : 사용자가 전달한 컬렉션을 순회하면서 전달한 함수가 만든 모든 값을 수집한다.
// yied_if : 첫 번째 인수로 지정된 조건자가 유효하다면 결과 범위에 값을 넣는다.
// 범위 내포 = filter와 결합된 transform이나 mbind
view::for_each(view::ints(1), [](int z) { // generage an infinite list of integers starting at 1
  return view::for_each(view::ints(1,z), [z](int y) {
      return view::for_each(view::ints(y,z), [y,z](int x) {
          return yield_if(
              x * x + y * y == z * z,
              std::make_tuple(x,y,z)
          );
      });
  });
});

// 오류 처리
// 모나드로 std::optional<T>
// 옵션을 사용하고자 한다면 값이 있는지 여부를 지속적으로 확인해야 한다.

std::optional<std::string> current_user_html() {
  if(!current_login) {
    return {};
  }

  const auto full_name = user_full_name(current_login.value());

  if(!full_name) {
    return {};
  }

  return to_html(full_name.value());
}

// mbind를 잘 정의한다면 이를 방지할 수 있다
template <typename T, typename F>
auto mbind(const std::optional<T>& opt, F f)
  -> decltype(f(opt.value())) {
    if(opt) {
      return f(opt.value());
    } else {
      return {};
    }
}

//이를 이용하면 위의 결과는
std::optional<std::string> current_user_html() {
  return mbind(
      mbind(current_login, user_full_name),
      to_html);
}

// 파이프로 바꾸면
std::optional<std::string> current_user_html() {
  return current | mbind(user_full_name)
                 | mbind(to_html);
}

// 모나드로 expected<T,E>
// expected 모나드 구성하면 오류가 무엇인지 알 수 있다.
template<typename T, typename E, typname F,
  typename Ret = typename std::result_of<F(T)>::type>
Ret mbind(const expected<T,E>& exp, F f) {
  if (!exp) {
    return Ret::error(exp.error()); //exp에 오류가 있으면 전달한다.
  }
  retrun f(exp.value()); //그렇지 않으면 f가 반환한 결과를 반환한다.
}

// 사용해보자
expected<std::string, int> user_full_name(const std::string& login);
expected<std::string, int> to_html(const std::string& text);

expected<std::string ,int> current_user_html() {
  return current_login | mbind(user_full_name)
                       | mbind(to_html);
}

// try 모나드
// expected 모나드에 예외를 사용하는 래핑 함수
template <typename F,
         typename Ret = typename std::result_of<F()>::Type,
         typename Exp = expected<Ret, std::exception_ptr>
Exp mtry(F f) { //f는 인수를 갖지 않는 함수다. 인수로 호출하려면 람다를 전달하면 된다.
  try {
    return Exp::success(f()); // 예외가 전혀 던져지지 않았다면 f를 호출한 결과가 들어있는 expected 인스턴스를 반환한다.
  }
  catch (...) {
    return Exp::error(std::current_exception()); // 예외가 던져진다면 예외에 대한 포인터가 있는 expected 인스턴스를 반환한다.
  }
}

// result 값이 되거나 던져진 예외에 대한 포인터가 된다.
auto result = mtry([=] {
  auto users = system.users();

  if (users.empty()) {
    throw std::runtime_error("No users");
  }

  return users[0];
});


// 이밖에 try의 사용 대신 함수가 예외에 대한 포인터를 가진 expected 인스턴스를 반환한다면 다음과 같이 expected 객체에 저장된 값을
// 반환하거나 보유한 예외를 던지는 함수를 만들 수 있다.
template <typename T>
T get_or_throw(const expected<T, std::exception_ptr>& exp) {
  if (exp) {
    return exp.value();
  } else {
    std::rethrow_exception(exp.error());
  }
}

// 모나드로 상태 처리
// 상태의 변경을 어떻게 처리할 것인가 
// 순수하지 않은 함수는 상태를 암시적으로 변경할 수 있다.
// 순수한 방법으로 상태를 변경하려면 모든 변경은 명시적으로 이루어져야한다.
// 가장 간단한 방법은 각 함수에 일반 인수와 함께 현재 상태를 전달하는 것이다.
// 즉 함수가 새로운 상태를 반환한다
// ex.)
template <typename T>
class with_log {
  public:
    with_log(T value, std::string log = std::string())
      : m_value(value)
      , m_log(log) {}

    T value() const {return m_value; }
    std::string log() const {return m_log;} 
  private:
    T m_value;
    std::string m_log;

}

with_log<std::string> user_full_name(const std::string& login);
with_log<std::string> to_html(const std::string& text);

// mbind로 로그 관리하기
template <typename T, typenameF>
typename Ret = typename std::result_of<F(T)>::type
Ret mbind(const with_log<T>val, F f) {
  // f를 사용해 주어진 값을 변환한다
  // 이렇게 하면 결과값과 f가 생성한 로그 문자열이 만들어진다.
  const auto result_with_log = f(val.value());
  // 결과 값을 반환해야 하지만 f가 반환한 로그만 반환하면 안된다.
  // 즉 이전 로그와 이 로그를 연결해야 한다.
  return Ret(result_with_log.value(),
      val.log() + result_with_log.log());
}

// 병행성과 연속 모나드 

