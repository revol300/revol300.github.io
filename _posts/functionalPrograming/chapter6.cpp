/* 지연 평가  */

// C++ 에서의 지연
// lazy va_val template class를 사용 (흔히 memoization 으로 부름)
// 다음과 같은 내용이 포함되어야 한다
// 계산
// 결과를 이미 계산했는지 나타내는 플래그
// 계산 값

// template<auto v> 는 c++17이상에서만 지원 lambda함수에 대한 타입 추론 지원 필요

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
//
// 컬렉션을 지연해 정렬


