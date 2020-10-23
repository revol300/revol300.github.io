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

// problem : scores가 변화되면 결과값에 문제가 생길 수 있다.
