// 대수적 데이터 유형과 패턴 일치

// 웹 페이지의 총 단어 수를 계산하는 애플리케이션
// 초기 상태 : 카운팅 처리가 아직 시작되지 않았다
// 카운팅 상태 : 웹 페이지가 구해지고 카운팅이 처리 중이다.
// 최종 상태 : 웹 페이지가 완전히 구해졌고 모든 단어가 카운트됐다.

struct state_t {
  bool started = false;
  bool finished = false;
  unsigned count = 0;
  socket_t web_page;
};
// 위의 구조는 문제가 있어 보인다.. started, finished 하나로.. count, web_page는 started, finished에 dependency 

// 대수적 데이터 유형
// 곱 ex.) std::pair, std::tuple
// 합 ex.) enum

// 상속을 통한 합 유형

// 상속을 통해 합 유형을 만들기 위한 태그가 붙여진 슈퍼클래스

class state_t {
  protected:
    // 이 클래스의 인스턴스는 생성 될 수 없으므로 생성자를 protected 함수로 만든다.
    // 생성자는 state+t를 상속한 클래스에서만 호출할 수 있다.
    state_t(int type)
      :type(type) {}
    // 각 하위클래스는 type 인수에 대해 다른 값을 전달해야 한다.
    // 이것을 사용해 dynamic_cast를 효율적으로 대체할 수 있다.
  public:
    virtual ~state_t() {};
    int type;
};

// 상이한 상태를 나타내는 유형
class init_t : public state_t {
  public:
    enum { id = 0 };
    // 초기 상태를 나타내는 클래스는 데이터를 가질 필요가 없다.
    // 현재까지는 웹 페이지에 대한 핸들러나 카운터를 갖고 있지 않다.
    // 유형을 ID(0)으로 설정하면 된다.
    init_t() : state_t(id) {}
};

class running_t : public state_t {
  public:
    enum { id = 1 };
    running_t() : state_t(id) {}

    unsigned count() const {
      return m_count;
    }
    ...
 private:
    unsigned m_count = 0;
    socket_t m_web_page;
    // 실행 상태의 경우 카운팅할 단어가 있는 웹 페이지의 핸들러와 카운터가 필요하다.
};

class finished_t : public state_t {
  public:
    enum { id = 2 };
    finished_t(unsigned count)
      : state_t(id), m_count(count) {}
    // 카운팅이 종료되면 더 이상 웹 페이지에 대한 핸들러가 필요하지 않다.
    // 계산된 값만 필요할 뿐이다.

    unsigned count() const {
      return m_count;
    }

  private:
    unsigned m_count;
};

// 프로그램은 이제 state_t 에 대한 포인터를 가지고, 상태가 변경되면 기존 인스턴스를 파기하고 새로운 인스턴스로 대체한다.

// 주요 프로그램

class program_t {
  public:
    program_t()
      : m_state(std::make_unique<init_t>()) {}
    ...

    void counting_finished() {
      assert(m_state->type == running_t::id);
      // 카운팅 상태에 있었는지를 확인

      auto state = static_cast<running_t*>(
          m_state.get());
      // m_state가 가리키는 클래스의 정확한 유형을 알고 있다.
      // 따라서 이에 대한 형 변환을 정적으로 할 수 있다.

      m_state = std::make_unique<finished_t>(
          state->count());
      // 종료 결과를 갖는 새 상태로 전환한다.
      // 이전 상태가 파기된다.
    }

  private:
    std::unique_ptr<state_t> m_state;
}

// 공용체와 std::variant를 통한 합 유형

class init_t {
};

class running_t {
  public:
    unsigned count() const {
      return m_count;
    }

    ...
 private:
    unsigned m_count=0;
    socket_t m_web_page;
};

class finished_t {
  public:
    finished_t(unsigned count)
      : m_count(count) {
      }
    unsigned count() const {
      return m_count;
    }

  private:
    unsigned m_count;
}

// std::variant를 사용한 주프로그램
class program_t {
public:
  program_t() : m_state(init_t()) {}

  void counting_finished() {
    auto* state = std::get_if<running_t>(&m_state);
    assert(state != nullptr);
    m_state = finished_t(state->count());
  }

private:
  std::variant<init_t, running_t, state_t> m_state;
}

// 특정 상태 구현
// 하나의 상태를 처리하는 논리를 그 상태를 정의하는 객체 안에 두고
// 상태 전이를 수행하는 논리를 주 프로그램에 배치

void count_words (const std::string& web_page) {
  assert(m_state.index() == 0);
  m_state = running_t(web_page);
  ... // 단어의 개수를 센다
  counting_finished();
}

void counting_finished() {
  const auto* state = std::get_if<running_t>(&m_state);
  assert(state != nullptr);
  m_state = finished_t(state->count());
}

class running_t {
  public:
    running_t(const std::string& url)
      : m_web_page(url) {

    }

    void count_words() {
      m_count = std::distance(
          std::istream_iterator<std::string>(m_web_page),
          std::istream_iterator<std:string>());
    }

    unsigned count() const {
      return m_count;
    }
  private:
    unsigned m_count = 0;
    std::istream m_web_page;
}

// 특수한 합 유형 : 옵션 값

struct nothing_t {};
template <typename T>
using optional = std::variant<nothing_t, T>;

// T이면 T를 주고 T가 아니면 nothing을 반환하므로 매우 유용하다

template <typename T, template Variant>
std::optional<T> get_if(const Variant& variant) {
  T* ptr = std::get_if<T>(&variant);

  if(ptr) {
    return *ptr;
  } else {
    return std::optional<T>();
  }
}

// 이에 맞추어 counting_finished 함수를 다시 구현하면

void counting_finished() {
  auto state = get_if<running_t>(m_state);
  assert(state.has_value());
  m_state = finished_t(state->count());
}

// get_if 는 무조건 포인터 혹은 std::optional<T>() 라는 값을 주기 때문에 has_value()함수 사용이 가능하다.

// 오류 처리를 위한 합 유형
// 위의 함수 예시에서 오류 처리는 고려되고 있지 않다. 이를 고려해보자
// T : 반환하려는 값의 유형
// E : 오류 유형

template<typename T, typename E>
class expected {
  private:
    union {
      T m_value;
      E m_error;
    };

    bool m_valid;
};

template<typename T, typename E>
class expected {
  ...
  T& get() {
    if (!m_valid) {
      throw std::logic_error("Missing a value");
    }
    return m_value;
  }

  E& error() {
    if(m_valid) {
      throw std::logic_error("There is no error");
    }
    return m_error;
  }

  // m_value 나 merror에 따라 소멸자 호출
  ~expected() {
    if (m_valid) {
      m_value.~T();
    } else {
      m_error.~E();
    }
  }

};

template<typename T, typename E>
class expected {
  ...
  template <typename... Args>
  static expected success(Args&&... params) {
    expected result;
    result.m_valid=true;
    new (&result.m_value)
      T(std::forward<Args>(params) ...);
    return result;
  }

  template <typename... Args>
  static expected error (Args&&... params) {
    expected result;
    result.m_valid = false;
    new (&result.m_error)
      E(std::forward<args>(params) ...);
    return result;
  }
};

// 위치 지정 (Placement) new
// 값을 위한 메모리를 할당하고 그 값을 초기화하는 일반적인 new 구문과 달리 위치 지정 new 구문을 사용하면 이미 할당된 메모리를 사용하고 그 안에 객체를 생송할 수 있다.
// expected<T,E> 경우 메모리는 사용자가 정의한 공용 멤버 변수에 의해 미리 할당된다.
// 이것은 자주 사용하는 기법은 아니지만 실행 시에 동적 메모리 할당을 수행하지 않는 합 유형을 구현하려는 경우 필요하다.

// expected<T,E>의 복사 및 이동 생성자

// 복사 생성자
expected(const expected& other) : m_valid(other.m_valid) {
  if(m_valid) {
    new (&m_value) T(other.m_value);
  } else {
    new (&m_error) E(other.m_error);
  }
}

// 이동 생성자
expected(expected&& other) : m_valid(other.m_valid) {
  if(m_valid) {
    new (&m_value) T(std::move(other.m_value));
  } else {
    new (&m_error) E(std::move(other.m_error));
  }
}

// 대입연산자는 다음과 같은 네 가지 경우에 동작해야 한다.
// 1. 복사되는 대상과 복사하려는 원본 둘 다 유효한 값을 갖는 경우
// 2. 두 인스턴스 모두 오류가 있는 경우
// 3. this 인스턴스는 오류를 갖고 other 인스턴스는 값을 갖는 경우
// 4. this 인스턴스는 값을 갖고 other 인스턴스는 오류를 갖는 경우

// copy-and-swap을 사용하여 구현

// expected<T,E>의 교환 함수
void swap(expected& other) {
  using std::swap;
  if (m_valid) {
    if(other.m_valid) {
      //둘 다 값이 있으면 그냥 swap
      swap(m_value, other.m_value); 
    } else {
      //other가 오류값이 있으면 오류값을 temp에 옮기고
      auto temp = std::move(other.m_error);
      //other 지운뒤
      other.m_error.~E();
      //value를 other로 옮기고
      new (&other.m_value) T(std::move(m_value));
      //value를 지운뒤
      m_value.~T();
      //temp를 error에 옮긴다
      new (&m_error) E(std::move(temp));
      //무사히 진행되면 valid 값 교환
      std::swap(m_valid, other.m_valid);
    }
  } else {
    if (other.m_valid) {
      // 순서만 바꿔서 다시 수행
      other.swap(*this);
    } else {
      // 둘다 오류이므로 그냥 swap
      swap(m_error, other.m_error);
    }
  }
}

expected& operator=(expeted other) {
  //other는 copy되어 swap 되므로 exception-safe하다
  swap(other);
  return *this;
}

// 형 변환 연산자 정의
operator bool() const {
  return m_valid;
}

// 형 변환 연산자 정의
operator std::optional<T>() const {
  if (m_valid) {
    return m_value;
  } else {
    return std::optional<T>();
  }
}

template <typename T, template Variant,
         template Expected = expected<T, std::string>>
Expected get_if(const Variant& variant) {
  T* ptr = std::get_if<T>(variant);
  if (ptr) {
    return Expected::success(*ptr);
  } else {
    return Expected::error("Variant doesn't contain the desired type");
  }
}

// 대수적 데이터 유형으로 도메인 모델링

