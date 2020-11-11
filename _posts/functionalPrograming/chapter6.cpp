/* 지연 평가  */

// C++ 에서의 지연
// lazy va_val template class를 사용 (흔히 memoization 으로 부름)
// 다음과 같은 내용이 포함되어야 한다
// 계산
// 결과를 이미 계산했는지 나타내는 플래그
// 계산 값

// 생성자에 대한 타입 추론 지원 필요 c++17이상에서만 지원
// https://www.devoops.kr/67
// 따라서 따로 템플릿 함수 구성이 필요

template <typename F>
class lazy_val {
private:
  F m_computation;
  mutable bool m_cache_initialized;
  mutable decltype(m_computation()) m_cache;
  mutable std::mutex m_cache_mutex;
public:
  lazy_val(F computation) :
    m_computation(computation),
    m_cache_initialized(false) {}
};

template <typename F>
inline lazy_val<F> make_lazy_val(F&& computation) {
  return lazy_val<F>(std::forward<F>(computation));
}

// 값은 어떤 방식으로?
// 1. lazy_val에 호출 연산자 ( operator() )를 정의해 함수 객체로 만든다
// 2. lazy_val instance가 일반 변수처럼 보이게 하는 형 변환 연산자를 생성
// 2번에 대한 ex

template<typename F>
class lazy_val {
  private:
    ...
  public:
      operator const decltype(m_computation())& () const {
        std::unique_lock<std::mutex> lock{m_cache_mutex};
        
        if(!m_cache_initialized) {
          m_cache = m_computation();
          m_cache_initialized = true;
        }
        return m_cache;
      }
};

//multithread에서 unique_lock을 매번 걸 필요는 없기 때문에 std::call_once를 사용하자

template<typename F>
class lazy_val {
  private:
    F m_computation;
    mutable decltype(m_computation()) m_cache;
    mutable std::once_flag m_value_flag;

  public:
    ...
    operator const decltype(m_computation())& () const {
      std::call_once(m_value_flag, [this] {
          m_cache= m_computation();
      });
      return m_cache;
    }
};

// 최적화 기법으로서의 지연

// 컬렉션을 지연해 정렬

// 상위 10개의 항목만이 필요한 sorting의 경우
// quick sort를 이용한다면 필요한 상위 k개의 항목에 대해서만 sorting을 수행
// 나머지 항목은 아무런 작업도 수행하지 않는다
// n개의 항목에 대해 k개의 상위 항목을 원한다면 복잡도는 O(n + klogk)

// 사용자 인터페이스의 항목 뷰

// 사용자에게 보이지 않는 데이터는 가져올 필요가 없다. 필요할 때 필요한 정보를 보여주기만 하면 된다.

// 함수 결과 캐싱에 의한 재귀 트리 프루닝

// 캐싱을 이용한 피보나치 구현
std::vector<unsigned int> cache{0,1};

unsigned int fib(unsigned int n) {
  if(cache.size() > n) {
    return cache[n]; // 값이 캐시에 이미 존재한다면 그 값을 반환한다.
  } else {
    const auto result = fib(n-1) + fib(n-2);
    // 결과를 가져와 캐시에 추가한다.
    // 모든 이전 값은 채워진 상태라는 것을 알고 있으므로  push_back을 사용해 n번째 항목을 추가할 수 있다.
    cache.push_back(result);
    return result;
  }
}
// 단점은 캐시에 너무 많은 메모리가 사용된다. 어차피 2개의 값만 사용되기 때문에 마지막 두개의 값만 기억하자

class fib_cache {
  public:
    fib_cache()
      : m_previos{0}
      , m_last{1}
      , m_size{2} {}

    size_t size() const {
      return m_size;
    }

    unsigned int operator[] (unsigned int n ) const {
      return n == m_size-1? m_last :
             n == m_size-2? m_previos :
                            0;
    }

    void push_back(unsigned int value) {
      m_size++;
      m_previous = m_last;
      m_last =value;
    }
};

// 지연 형태의 동적 프로그래밍
// 레벤슈타인 거리를 재귀적으로 계산하기
// 문자열 길이가 m, n인 경우에 대해
unsigned int lev(unsigned int m, unsigned int n) {
  return m == 0? n // 하나의 문자열이 비어 있다면 거리는 다른 한 문자열의 길이가 된다.
        :n == 0? m
        :std::min ({
          lev(m-1, n) +1, // 문자를 추가하는 동작을 센다
          lev(m, n-1) +1, // 문자를 제거하는 동작을 센다
          lev(m-1, n-1) + (a[m-1] != b[n-1]) // 문자를 대체하는 동작을 세지만 동일한 문자로 대체하지 않는 경우로만 한정한다
        });
}
// 이것도 지연 연산을 위해 결과를 캐시하자

// 일반화된 메모이제이션
template <typename Result, typename... Args>
auto make_memoized(Result (*f)(Args...)) {
  std::map<std::tuple<Args...>, Result> cache;

  return [f, cache](Args... args) mutable -> Result {
    const auto args_tuple = std::make_tuple(args...);
    // 해당 결과가 캐시됐는지를 검사
    const auto cached = cache.find(args_tuple);

    if (cacehd == cached.end()) {
      // 캐시 미스가 발생하는 경우 함수를 호출하고 결과를 캐시에 저장
      auto result = f(args...);
      cache[args_tuple] = result;
      return result;
    } else {
      return cached->second;
    }
  }
}

auto fibmemo = make_memoized(fib);
std::cout <<"fib(15)= " << fibmemo(15) << std::endl;
std::cout <<"fib(15)= " << fibmemo(15) << std::endl;

// 문제점  재귀 함수의 경우 캐시가 제대로 동작하지 않는다
// fib 수정이 필요
template <typename F>
unsigned int fib(F&& fibmemo, unsigned int n) {
  return n == 0 ? 0
       : n == 1 ? 1
       : fibmemo(n-1)+fibmemo(n-2);
}
// 너무 복잡함;;

// 재귀 함수에 대한 메모이제이션 래퍼
class null_param {};

template <class Sig, class F>
class memoize_helper;

template <class Result, class... Args, class F>
class memoize_helper<Result(Args...), F> {
  private:
    using function_type = F;
    using args_tuple_type = std::tuple<std::decay_t<Args>...>;

    function_type f;
    mutable std::map<args_tuple_type, Result> m_cache;
    mutable std::recursive_mutex m_cache_mutex;

  public:
    template <typename Function>
      memoize_helper(Function&&f, null_param) : f(f) {}
      memoize_helper(const memoize_helper& other) : f(other.f) {}

      template <class... InnerArgs>
      Result operator()(InnerArgs&&... args) const {
        std::unique_lock<std::recursive_mutex> lock{m_cache_mutex};

        const auto args_tuple = std::make_tuple(args...);
        const auto cached = m_cache.find(args_tuple);

        if (cached != m_cache.end()) {
          return cached->second;
        } else {
          auto&& result = f(
              *this,
              std::forward<InnerArgs>(args)...);
          m_cache[args_tuple] = result;
          return result;
        }
      }
};

template <class Sig, class F>
memoize_helper<Sig, std::decay_t<F>> make_memoized_r(F&& f) {
  return {std::forward<F>(f), detail::null_param()};
}

//fib의 메모화된 버전 create
auto fibmemo = make_memoized_r<unsigned int(unsigned int)>(
    [](auto& fib, unsigned int n) {
      std::cout << "Calculating" << n << "!\n";
      return n == 0 ? 0
           : n == 1 ? 1
           : fib(n-1)+fib(n-2);
  }
);

// 식 템플릿과 지연 문자열 연결
std::string fullname = title + " " + surname + "," + name;

std::string fullname = (((title+ " ") + surname) + ",") + name;

// 각 연산을 수행할 때마다 새로 메모리를 할당하고 복사하는 작업이 수행
// string의 크기를 먼저 알고 메모리 할당후에 그때 복사를 하면 좋지 않을까?

template <typename... Strings>
class lazy_string_concat_helper;

template <typename LastString, typename... Strings>
class lazy_string_concat_helper<LastString, Strings...> {
  private:
    LastString data;
    lazy_string_concat_helper<Strings...> tail;

  public:
    lazy_string_concat_helper( LastString data, lazy_string_concat_helper<Strings...> tail)
      : data(data)
      , tail(tail) {}

    int size() const {
      return data.size() + tail.size();
    }

    template <typename It>
    void save(It end) const {
      const auto begin = end - data.size();
      std::copy(data.cbegin(), data.cend(), begin);
      tail.save(begin);
    }

    //implicit conversion
    operator std::string() const {
      std::string result(size(), '\0');
      save(result.end());
      return result;
    }

    lazy_string_concat_helper<std::string, LastString, Strings...>
      operator+(const std::string& other) const {
        return lazy_string_concat_helper <std::string, LastString, Strings...> (other, this);
    }
}

// 재귀함수 이므로 기본 경우 생성 필요
template <>
class lazy_string_concat_helper<> {
  public:
    lazy_string_concat_helper() {}
    int size() const { return 0;}
    template <typename It> void save(It) const {}

    lazy_string_concat_helper<std::string> operator+(const std::string& other) const {
      return lazy_string_concat_helper<std::string>(
        other,
        *this
      );
    }
};


// 사용 예시
lazy_string_concat_helper<> lazy_concat;

int main(int argc, char* argv[]) {
  std::string name = "Jane";
  std::string surname = "Smith";

  const std::string fullname = 
    lazy_concat + surname + "," + name;
  std::cout << fullname << std::endl;
}

// 순수성과 식 템플릿
// lazy_string_concat_helper 클래스에 원본 문자열의 복사본을 저장 => 원본 문자열에 대한 참조를 사용하는 것이 더 효율적
template <typename LastString, typename... Strings>
class lazy_string_concat_helper<LastString, Strings...> {
  private:
    const LastString& data;
    lazy_string_concat_helper<Srings...> tail;

  public:
    lazy_string_concat_helper(
        const LastString& data,
        lazy_string_concat_helper<Strings...> tail)
      : data(data)
      , tail(tail) {}
    ...
};

