// 범위

// ex std::partition을 여러번 사용하여 팀별로 사람들을 분리하는 함수
template <typename Persons, typename F>
void group_by_name(Persons& persons, F team_for_person, const std::Vector<std:string>& teams) {
  auto begin = std::begin(persons);
  const auto end = std::end(persons);

  for (const auto& team : teams) {
    begin = std::partition(begin, end,
        [&](const auto& person) {
          return team == team_for_person(person);
        }
    );
  }
}

// 문제점 STL 알고리즘은 컬렉션에 대해서 begin과 end를 인자로 받는다 즉
// 1. 알고리즘은 컬렉션을 결과로 반환할 수 없다
// 2. 컬렉션을 반환하는 함수가 있더라도 임시변수를 생성해서 begin end의 형태로 전달해야한다
// 3. 이러한 이유로 컬렉션을 새로 반환하기보다는 자신의 인수를 직접 변경하는 방식을 취한다

// 범위 : 두개의 반복자를 사용
// 하나는 컬렉션의 첫 요소를 가리키고 다른 하나는 마지막 요소의 바로 다음 항목을 가리킨다
// Ex
std::vector<std::string> names =
  transform(filter(people, is_female), name);

std::vector<std::string> names = people | filter(is_female)
                                        | transform(name);

// 데이터에 대한 읽기 전용 뷰 만들기

//범위 용도의  filter 함수
//filter는 std::vector와 같은 컬렉션을 반환하는 대신 begin 반복자가 스마트 프록시 반복자 (주어진 조건자를 만족하는 소스 컬렉션의 첫 요소를 가리키는)가 되는 범위 구조체를 반환한다.
//필터링 프록시 반복자에 대한 증가 연산자
auto& operator++() {
  ++m_current_position; // 필터링 중인 컬렉션에 대한 반복자. 프록시 반복자가 증가돼야 할 때 조건자를 만족하는 현재 요소 뒤의 첫 요소를 찾는다.
  m_current_position =
    std::find_if(m_current_position, // 다음 요소에서 탐색을 시작한다.
                 m_end, // 더 이상의 요소가 조건자를 만족하지 않는다면 소스 컬렉션의 끝(필터링된 범위의 끝 부분이기도 하다)을 가리키는 반복자를 반환한다.
                 m_predicate);
  return *this;
}

//요약하면 필터링 시 iterator를 조건에 부합하는 애들로 재구성된 프록시 iterator를 만든다

// 범위 용도의 transform 함수
// 그냥 그대로 다 보여주는데 변화만 적용하면 된다
// ex transform proxy iterator를 위한 역참조 연산자

auto operator*() const {
  return m_function(
        *m_current_position
      );
}

// 범위 값의 지연 평가
// 

std::vector<std::string> names = people | filter(is_female)
                                        | transform(name)
                                        | take(3);
// 위 코드에 대한 분석
