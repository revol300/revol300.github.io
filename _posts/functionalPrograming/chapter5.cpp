/* 순수성: 가변 상태 회피 */

/* 가변 상태의 문제*/
class movie_t {
  public:
    double average_score() const;

  private:
    std::string name;
    std::list<int> scores;
}

// calculate average score of movies
double movie_t::average_score() const {
  retrun std::accumulate(scores.begin(), scores.end(), 0)/(double)scores.size();
}

// problem : scores가 변화되면 결과값에 문제가 생길 수 있다
// 1) 평균 점수를 계산하는 중에 누군가 목록을 변경할 수 있다
// 2) 변경되지 않더라도 movie_t 생성시에 이 갑들이 초기화 되어야 하는데 이건 디버깅이 상대적으로 쉽다
// 변수의 허용을 금지하면된다 => 현실적으로 쉬운 방향은 아니다 => 대신 변경과 부작용을 최소한으로 유지하면서 소프트웨어를 설계하자

// 참조 투명성 (referenctial transparency) : 어떤 표현식이 있고 해당 표현식의 결과로 전체 표현식을 대체해도 프로그램이 전혀 다르게 동작하지 않는 것을 의미
// 즉 같은 argument에 대해 같은 result를 내뱉어야 한다

double max(const std::vector<double>& numbers) {
  assert(!numbers.empty());
  auto result = std::max_element(numbers.cbegin(),numbers.cend());
  std::cerr << "Maximum is : " << *result << std::endl;
  return * result;
}

int main() {
  auto sum_max = max({1}) + max({1,2}) + max({1,2,3});
  std::cout << sum_max << std::endl;
}

// std::cerr라는 출력값이 있기 때문에 max함수는 참조적으로 투명하지 않고 또한 수수하지 않다
// std::cerr를 제거하면 max는 순수하다
// 다만 시점에 따라 std:err를 무시할 수 있는 것이라 생각한다면 순수하다고 간주 할 수 있다.

// 부작용이 없는 프로그래밍
// 값을 변경하지 않는 대신에 값을 새로 만드는 방법을 사용하자
// cacluate player's next position
position_t next_position(direction_t direction, const position_t& previos_position, const maze_t& maze) {
  const position_t desired_position{previos_position, direction};

  return maze.is_wall(desired_position)? previos_position : desired_position;
}

position_t::position(const position_t& original,
                    direction_t direction) :
                     x { direction == Left ? original.x -1 :
                         direction == Right ? original.x+1 : original.x},
                     y { direction == Up ? original.y+1 :
                         direction == Down ? original.y-1 : original.y} {}

void draw_player(const position_t& position, direction_t direction);

void process_events(const maze_t& maze, const position_t& current_position) {
  if (maze.is_exit(current_position)) {
    std::cout << "Exit !" << std::endl;
    return;
  }
  
  const direction_t direction = ...; // user input
  draw_maze();
  draw_player(current_position, direction);

  const auto new_position = next_position(
      direction,
      current_position,
      maze);
  process_events(maze, new_position);
}

int main() {
  const maze_t maze("maze.data");
  process_events(
      maze,
      maze.start_poistion();
      );
}

// 물론 복사를 통해 전달하는 것이 비효율적일 수 있기 때문에 항상 새로운 copy를 만드는 대신 변경할 내용만을 반영하여 함수를 모델링하면 더욱 효율적으로 코드를 구성할 수 있다.

// parallel 환경에서의 가변과 불변 상태
// 가변 상태의 문제점

void client_disconnected(const client_t& client) {
  // 클라이언트가 사용한 자원을 해제한다.
  ...
  // 연결된 클라이언트의 수를 감소시킨다.
  connected_clients--;

  if (connected_clients == 0 ) {
    //절전 모드로 진입한다.
  }
}
// => 동시 접근시 문제 발생 => 해당 문제의 해결책으로 lock을 사용하여 동시 접근을 차단
// 문제의 원인
// 1. 불변 데이터를 가지고 이를 공유하지 않는다 => 문제 없음
// 2. 가변 데이터를 가지고 이를 공유하지 않는다 => 문제 없음
// 3. 불변 데이터를 가지고 이를 공유한다 => 문제 없음
// 4. 가변 데이터를 가지고 이를 공유한다 => 문제 발생
// 해당 문제는 10장과 12장에서 더 자세히 다룰 예정

// const 화의 중요성
// C++에서는 const와 constexpr키워드를 통해 가변성을 제한한다.
const std::string name{"John Smith"};

std::string name_copy = name;
std::string& name_ref = name; //error
const std::string& name_constref = name;
std::string* name_ptr = &name; //error
const std::string* name_constptr = &name;

void print_name(std::string name_copy);
void print_name_ref(std::string& name_ref); // error when called
void print_name_constref(const std::string& name_constref);

class person_t {
  public:
    std::string name_nonconst();
    std::string name_const() const;

    void print_nonconst(std::ostream& out);
    void print_const(std::ostream& out) const;
};

// 위의 class별 멤버함수는 다음과 같다.
std::string person_t_name_nonconst(person_t& this);
std::string person_t_name_const(const person_t& this);
std::string person_t_print_nonconst(person_t& this, std::ostream& out);
std::string person_t_print_const(const person_t& this, std::ostream& out);

// 논리적 상수성과 내부 상수성
class person_t {
  public:
    const std::string name;
    const std::string surname;
};

// 이동 생성자와 이동 대입 연산자가 없어짐
/
class person_t {
  public:
    std::string name() const;
    std::string surname() const;

  private:
    std::string m_name;
    std::string m_surname;
};

// 논리적 상수성 (logical const-ness) : 사용자가 볼 수 있는 객체 데이터는 절대 변경되지 않는다
// 내부 상수성 (internal const-ness) : 객체 내부 데이터에 대한 변경이 없음
// 논리적 상수성은 지키고 내부 상수성을 지키지 않는 경우 => ex.) caching

class person_t {
  public:
    employment_history_t employment_history() const {
      std::unique_lock<std::mutex>
        lock{m_employment_history_mutex};

      if(!m_employment_history.loaded()) {
        load_employment_history();
      }
      return m_employment_history;
    }
  private:
    mutable std::mutex m_employment_history_mutex;
    mutable employment_history_t m_employment_history;
}

// 위 class는 외부에서는 불변이지만 내부 데이터를 변경해야 하는 class를 구현하는 일반적인 패턴이다.
// 이는 class data가 변경되지 않고 유지되거나 모든 변경이 동기화가 이뤄지는 상수 멤버 함수가 필요로 한다

// 임시 값에 대한 멤버 함수 최적화

person_t new_person {
  old_person.with_name("Joanne").with_surname("Jones");
};

class person_t {
  public:
    //lvalue에 대해 작동하는 버전
    person_t with_name(const std::string& name) const & {
      person_t result(*this);
      result.m_name = name;
      return result;
    }

    //rvalue에 대해 작동하는 버전
    person_t with_name(const std::string& name) && {
      person_t result(std::move(*this));
      result.m_name=name;
      return result;
    }
};

// 임시 객체의 복사본을 만들지 않고서도 코드를 최적화 => 현저한 개선이 있을 때에만 사용

// const 주의 사항
// 객체 이동을 차단하는 const
person_t some_function() {
  const person_t result;
  // 결과를 반환하기 전에 해야 할 작업
  return result;
}

person_t person = some_function();

//위와 같은경우 복사가 일어난다

// 얕은 const
class company_t {
  public:
    std::vector<std::string> employees_names() const;
  private:
    std::vector<person_t*> m_employees;
};

// 포인터에 대한 const만 적용되고 vector의 pointer가 가리키는 값의 변경에는 영향을 주지 않는다.

//* propagate_const wrapper를 사용하면 이러한 문제를 해결할 수 있다
