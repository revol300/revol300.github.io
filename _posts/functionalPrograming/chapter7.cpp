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
// sentinel : 반복자가 끝가지 왔는지를 검사하는데 사용하는 특별한 값 딱히 정해저 있는 것이 아니다
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
// 오른쪽 왼쪽을 다 봐야하는 것에서 비효율적인 연산 
// 컬렉션의 끝이 반복자이어야 한다는요건을 제거한다면 훨씬 쉽게 작성이 가능하다
// 오른쪽과 왼쪽을 비교하여 다르면 끝

// 센티넬로 무한 범위 만들기

// 센티널 접근법을 사용하면 무한 범위를 쉽게 만들 수 있다. 

// 처음 10개 영화와 그 순위를 출력하기
template<typename Range>
void write_top_10(const Range& xs) {
  auto items = 
    view::zip(xs, view::ints(1)) // 1에서 시작하는 정수 범위로 영화범위를 압축한다.
      | view::transform([](const auto& pair) { // transform 함수는 쌍을 받아서 영화 순위와 이름을 갖는 문자열을 만든다
        return std::to_string(pair.second) + " " + pair.first;
      })
      | view::take(10); // 처음 10편의 영화에만 관심이 있다.
  for (const auto& item : items) {
    std::cout << item << std::endl;
  }
}

// view::zip 함수는 두개의 범위를 받아서 이들 범위로 부터 항목의 쌍을 만드는 함수이다.
// 무한 범위의 또 다른 이점은 이들을 사용하는 거싱 아니라 이들에 동작할 수 있는 코드를 설계할 수 있고 이를 통해 크기를 알수 없는 범위뿐만 아니라 어떤 크기의 범위에서든 동작하는 코드를 만들 수 있다는 것이다.

// 단어 빈도 계산에 범위 사용
std::vector<std::string> words = 
  istream_range<std::string>(std::cin)
  | view::transform(string_to_lower) // 모든 단어를 소문자화한다
  | view::transform(string_only_alnum) // 문자와 숫자만 유지한다
  | view::remove_if(&std::string::empty); // 토큰에 문자나 숫자가 하나도 없다면 빈 문자열을 결과로 받을 수 있다. 이들은 건너 뛰자

std::string string_to_lower(const std::string& s) {
  return s | view::transform(tolower);
}

std::string string_only_alnum(const std::string &s) {
  return s | view::filter(isalnum);
}
