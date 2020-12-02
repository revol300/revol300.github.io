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
// 1. people | filter(is_femal) 이 평가될  때 새 뷰가 생성되는 것 외에는 아무 일도 발생하지 않는다.
// is_femail 조건자를 만족하는 첫 번째 항목을 가리키는 소스 컬렉션에 대한 반복자를 잠재적으로 초기화하는 것을 제외하면 people 컬렉션에서 한 사람도 접근하지 않았따.
// 2. 이 뷰를 | transform(name)에 전달한다. 발생하는 유일한 것은 새로운 뷰가 만들어진다는 점이다.
// 여전히 한 사람도 접근하지 않았으며 이들 중의 그 누구한테도 name 함수를 호출하지 않았다.
// 3. take(3)을 이 결과에 적용한다. 여전히 새로운 뷰만 생성될 뿐이다.
// 4. | take(3) 변환의 결과로 얻은 뷰에서 문자열 벡터를 만들어야 한다
// 이제 뷰가 다 결정된 상태에서 다음과 같은 일이 일어난다
// 1. take에 의해 반환되는 범위 뷰에 속하는 프록시 반복자에 대한 역참조 연산자를 호출한다
// 2. take에 의해 생성된 프록시 반복자는 요청을 transform에 의해 생성된 프록시 반복자로 전달한다. 이 반복자는 요청을 전달한다.
// 3. filter 변환에 의해 정의된 프록시 반복자를 역참조하려고 한다. 이것은 소스 컬렉션을 거치고 is_female 조건자를 만족하는 첫 번째 사람을 찾아 반환한다.
// 이번이 컬렉션 내의 사람에 처음 접근하는 것이며, 처음으로 is_female 함수를 호출하는 것이다.
// 4. filter 프록시 반복자를 역참조함으로써 얻어지는 사람은 name 함수로 전달되고 그 결과는 names 벡터에 삽입되도록 전달하는 take 프록시 반복자로 반환된다.

// 범위를 통한 값 변경
// 많은 유용한 변환이 간단한 뷰로 구현될 수 있지만 일부는 원본 컬렉션을 변경한다.
// 이들 변환을 뷰에 상반되는 액션이라 한다. ex) 정렬
// ex.) 단어를 읽어서 중복없이 순서대로 출력하는 함수
std::vector<std::string words = 
  read_text() | action::sort
              | action::unique;
// action::unique대신에 view::unique를 사용할 수도 있지만 view::unique는 연속해서 나타나는 반복되는 모든 값을 건너뛰는 뷰를 생성하고 실제 컬렉션에 대한 변화는 주지 않는다.

// 제한 범위와 무한 범위의 사용
// sentinel : 반복자가 끝가지 왔는지를 검사하는데 사용하는 특별한 값
// 입력 범위 처리 최적화에 제한 범위 사용
std::accumulate(std::istream_iterator<double>(std::cin),
                std::istream_iterator<double>(),
                0);

template <typename T>
bool operator==(const std::istream_iterator<T>& left,
    const std::istream_iterator<T>& right) {
  if(left.is_sentinel() && right.is_sentinel()) {
    return true;
  } else if (left.is_sentinel()) {
    //센티넬 조건자가 right 반복자에 대해
    //true 인지를 검사한다.
  } else if (right.is_sentinel()) {
    // 센티넬 조건자가 left 반복자에 대해
    // true 인지를 검사한다.
  } else {
    // 두 반복자 모두가 정상적인 반복자이며,
    // 이들 반복자가 컬렉션 내의 동일한 지점을
    // 가리키는지를 검사한다.
  }
}
