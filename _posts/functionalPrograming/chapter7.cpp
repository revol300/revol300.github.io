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

