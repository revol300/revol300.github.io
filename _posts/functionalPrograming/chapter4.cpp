/* ex1. */

class greater_than {
  public:
    greater_than(int value) : m_value(value) {

    }

    bool operator()(int arg) const {
      return arg > m_value;
    }

  private:
    int m_value;
};

greater_than greater_than_42(42);
greater_than_42(1); //false
greater_than_42(50); //true

std::partition(xs.begin(), xs.end(), greater_than(6));

/* ex2 */
template <typename Function, typename SecondArgType>
class partial_application_bind2nd_impl {
  public:
    partial_application_bind2nd_impl(Function function,
        SecondArgType second_arg)
      : m_function(function),
        m_second_arg(second_arg) {}

    template <typename FirstArgType>
    auto operator()(FirstArgType&& first_arg) const -> decltype(m_function(std::forward<FirstArgType>(first_arg), m_second_arg)) {

      return m_function(
          std::forward<FirstArgType>(first_arg),
          m_second_arg);
    }

  private:
    Function m_function;
    SecondArgType m_second_arg;
};

template <typename Function, typename SecondArgType>
partial_application_bind2nd_impl<Function, SecondArgType> bind2nd(Function&& function, SecondArgType&& second_arg) {
  return partial_application_bind2nd_impl<Function, SecondArgType>(
      std::forward<Function>(function),
      std::forward<SecondArgType>(second_arg));
}

auto bind_greater_than_42 = bind2nd(std::greater<int>(), 42);
bind_greater_than_42(1);
bind_greater_than_42(50);

std::partition(xs.begin(), xs.end(), bind2nd(std::greater<int>(), 6));

/* ex3 degree to radian */

std::vector<dobule> degrees = {0,30,45,60};
std::vector<dobule> radians(degrees.size());

#define PI 3.141592
std::transform(degrees.cbegin(), degrees.cend(), radians.begin(), bind2nd(std::multiplies<double>(), PI/180));


/* ex4 std::bind */

auto bound = std::bind(std::greater<double>(), 6, 42);
bool is_6_greater_than_42 = bound();

auto is_greater_than_42 = std::bind(std::greater<double>(), _1, 42);
auto is_less_than_42 = std::bind(std::greater<double>(), 42, _1);

is_less_than_42(6); //true
is_greater_than_42(6); //false

std::sort(scores.begin(), scores.end(), std::bind(std::greater<double>(), _2, _1));

class person_t {
  void print(std::osteram& out, output_format_t format) const{}
}

std::for_each(people.cbegin(), people.cend(), std::bind(&person_t::print, _1, std::ref(std::cout), person_t::name_only));

/* ex5 use lambda expression */

auto bound = [] {
  return std::greater<double>()(6,42);
};

auto is_greater_than_42 = [](double value) {
  return std::greater<double>()(value, 42);
};

auto is_less_than_42 = [](double value) {
  return std::greater<double>()(42,value);
};
is_less_than_42(6); //return true
is_greater_than_42(6); //return false

/* ex6 currying  */

// greater : (double, double) -> bool
bool greater(double first, double second){
  return first>second;
}

// greater_curried : double -> (double->bool)
auto greater_curried(double first) {
  return [first](double second) {
    return first>second;
  };
}

// Invocation
greater(2,3); // return false
greater_curried(2); //return function
greater_curried(2)(3);

/* ex7 database_access */
result_t query(connection_t& connection, session_t& session, const std::string& table_name, const std::string& filter);

auto table = "Movies";
auto filter = "Name = \"Sintel\"";

results = query(local_connection, session, table, filter);

auto local_query = query(local_connection);
auto remote_query = query(remote_connection);

results = local_query(session, table, filter);
auto main_query = query(local_connection, main_session);
results = main_query(table, filter);
auto movies_query = main_query(table);
results = movie_query(filter);

// https://medium.com/better-programming/functional-programming-currying-vs-partial-application-53b8b05c73e3

/* ex8 lifting */
// 특정 타입을 다루는 함수를 특정 타입과 관련된 다른 타입을 다루는 함수로 변화시키는 것

void pointer_to_upper(std::string* str) {
if(str)
  to_upper(*str);
}

void vector_to_upper(std::vector<std::string>& strs) {
  for (auto& str : strs) {
    to_upper(str);
  }
}

void map_to_upper(std::map<int, std::string>& strs) {
  for (auto& pair : strs) {
    to_upper(pair.second);
  }
}

template<typename Function>
auto pointer_lift(Function f) {
  return [f](auto* item) {
    if (item) {
      f(*item);
    }
  }
}

template<typename Function>
auto collection_lift(Function f) {
  return [f](auto& itmes) {
    for (auto& itme: itmes){
      f(item);
    }
  }
}

template<
  typename C,
  typename P1 = typename std::remove_cv<
    typename C::value_type::first_type>::type,
  typename P2 = typename C::value_type::second_type
>
std::vector<std::pair<P2,P1>> reverse_pairs(const C& itmes) {
  std::vector<std::pair<P2,P1>> result(items.size());

  std::transform(
      std::begin(items), std::end(items),
      std::begin(result),
      [](const std::pair<const P1,P2>& p){
      return std::make_pair(p.second, p.first);
      }
  );
  return result;
}

/* summary */
std::bind 같은 고차원 함수는 기존의 함수를 새로운 함수로 변환하는 데 사용할 수 있다. std::bind 를 이용해 n개의 인수를 가진 함수를 std::parition과 std::sort 같은 알고리즘에 전달할 수 있는 단항 또는 이항 함수로 쉽게 변환할 수 있다.
함수의 어떤 인수가 바인딩되지 말아야 할지를 정의할 때 플레이스 홀더는 높은 수준의 표현력을 제공한다. 이들은 원본 함수의 인수의 순서를 바꾸고 같은 인수를 여러 번 전달하는 등의 작업을 가능하게 해준다.
std::bind는 부분 함수 애플리케이션 작업에 간결한 구문을 제공할지라도 성능 불이익이 존재할 수 도 있다. 자주 호출되는 성능이 중요한 코드를 작성한다면 람다의 사용을 고려 할 수도 있다. 람다는 좀 더 장황하지만 컴파일러는 std::bind에 의해 생성된 함수 객체보다 이들을 더 잘 최적화할 수 있다. 자체적인 라이브러리를 위한 API를 정의하는 것은 어려운 일ㅇ이다. 많은 사용사례를 다뤄야 한다. 커리 함수를 사용하는 API를 만드는 것을 고려해보자
함수형 프로그래밍에 관해 자주 들었던 말 중 하나는 이것이 좀 더 짧은 코드를 작성하게 해준다는 것이었다. 쉽게 결합할 수 있는 함수를 만든다면 일반적인 명령형 프로그래밍 접근법이 사용하는 코드의 일부분으로 복잡한 문제를 해결 할 수 있다. 
