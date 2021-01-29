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


