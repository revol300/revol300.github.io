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

