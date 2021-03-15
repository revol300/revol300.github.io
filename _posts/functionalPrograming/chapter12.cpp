// 병행성 시스템을 위한 함수적 설계

/* 소프트웨어 개발에서 결함 중 상당 부분은 코드가 실행될 수 있는 모든 가능한 상태를 프로그래머가 완전히 이해하지 못한 점에 기인한다.
 * 멀티스레드 환경에서 이해가 부족하면 주의를 기울이더라도 공황 상태에 이를 지경까지 문제의 결과는 증폭된다.
 * - 존 카맥 */

// 액터 모델 : 구성 요소로 사고

/* 작업을 수행하는 데 필요한 정보를 요청하지 말고 작업을 수행하기 위한 정보를 가진 객체를 요청하자.
 * 알렌 홀룹 */

// 액터 모델에서 액터는 아무것도 공유하지 않고서 메시지를 서로 보낼 수 있는 완전히 분리된 개체다.
// 엑터 클래스가 가져야 하는 최소한의 것은 메시지를 주고받는 방법이다.
// - 액터는 단일 유형의 메시지를 수신하고 단일 유형의 메시지를 전송할 수 있다.
// - 각 액터가 메시지를 보낼 대상을 선택할 수 있게 하는 대신에 함수형 방식으로 액터를 구성할 수 있도록 외부 컨트롤러에 이 선택안을 남겨둔다
//   외부 컨트롤러는 액터가 listen해야 할 소스를 스케줄 한다
// - 어떤 메시지를 비동기적으로 처리해야 하는지, 어떤 메시지를 처리하지 않아야 하는지를 결정하는 것을 외부 컨트롤러에 맡긴다.
//
// 액터의 최소 인터페이스
template <typename SourceMessageType,
         typename MessageType> // 액터는 한 유형의 메시지를 수신하고 다른 유형의 메시지를 보낼 수 있다.
class actor {
public:
  using value_type = MessageType; // 액터가 전송할 메시지의 유형을 정의한다. 따라서 나중에 액터에 연결해야 할 때 이를 확인할 수 있다.
  
  void process_message(SourceMessageType&& message); // 도착하는 새 메시지를 처리한다.

  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit); // 엑터가 메시지를 보내고자 할 때 호출하는 m_emit 핸들러를 설정한다
private:
  std::function<void(MessageType&&)> m_emit;
}

// 간단한 메시지 소스 만들기
// 북마크 처리 앱
// 입력 : {"FirstURL": "https://isocpp.org/", "Text": "Standard C++" }
// 네트워크 통신 라이브러리 : Boost.Asio (http://mng.bz/d62x)
// JSON 작업 : https://github.com/nlohmann/json
// 클라이언트 연결을 listen하는 서비스
class service {
public:
  using value_type = std::string; // 클라이언트의 입력을 한 줄씩 읽는다. 보낸 메시지는 문자열이다.

  explicit service(
      boost::asio::io_service& service,
      unsigned short port = 42042)
    : m_acceptor(service,
        tcp::endpoin(tcp::v4(), port)) // 지정된 포트에서 listen하는 서비스를 만든다.
      , m_socket(service) {}

  service(const service& other) = delete; // 복사 비활성화
  service(service&& other) = default; // 이동 허용

  // 누군가가 message_service로부터 메시지를 리슨하는 것을 등록할때까지 클라이언트로부터의 연결을 수락하지 않는 지점
  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    m_emit = emit;
    do_accept();
  }

private:
  void do_accept() {
    m_acceptor.async_accept(
        m_socket,
        [this](const error_code& error) {
          if(!error) {
          // 들어오는 클라이언트에 대한 세션을 만들고 시작한다.
          // 세션 객체가 클라이언트에서 메시지를 읽으면 m_emit으로 전달된다.
          // make_shared_session은 세션 객체 인스턴스에 대한 공유 포인터를 생성한다.
            make_shared_session(
                std::move(m_socket),
                m_emit
                )->start();
          } else {
            std::cerr << error.message() << std::endl;
          }
          // 다른 클라이언트를 listen한다.
          do_accept();
        });
  }

  tcp::acceptor m_acceptor;
  tcp::socket m_socket;
  std::function<void(std::string&&)> m_emit;
};

// 위 코드에 대해서 요약하면
// m_acceptor.async_accept는 새로운 클라이언트가 나타날 때 실행되도록 전달된 람다를 스케줄한다.
// 람다는 클라이언트가 성공적으로 연결됐는지 확인한다. 그렇다면 클라이언트에 대한 새로운 세션을 만든다.
// 여러 클라이언트를 받아들일 수 있기를 원하면 do_accept를 다시 호출한다.

// 세션 객체는 자신의 수명을 관리해야한다. 오류가 발생하면 세션은 즉시 자신을 파기해야한다.
// 이를 위해 enable_shared_from_this를 사용한다

// 메시지 읽기와 내보내기
tempalte <typename EmitFunction>
class session: public std::enable_shared_from_this<session<EmitFunction>> {
  public:
    session(tcp::socket&& socket, EmitFunction emit)
      : m_socket(std::move(socket))
      , m_emit(emit) {}

    void start() {
      do_read();
    }

  private:
    // 이 세션의 소유권을 공유하는 다른 포인터를 만든다.
    using shared_session = 
      std::enable_shared_from_this<session<EmitFunction>>;

    void do_read() {
      auto self = shared_session::shared_from_this();

      boost::asio::async_read_until(
          m_socket, m_data, '\n', // 입력에서 개행 문자에 도달할 때 실행될 람다를 스케줄 한다.
          [this, self](const error_code& error,
            std::size_t size){

            if(!error) {
              std::istream is(&m_data);
              std::string line;
              std::getline(is, line);
              m_emit(std::move(line));
              do_read();
            }

          });
    }

    tcp::socket m_socket;
    boost::asio::streambuf m_data;
    EmitFunction m_emit;
};

// 반응형 스트림을 모나드로 모델링
// 반응형 스트림은 모나드인가?
// 모나드가 되기 위해 필요한 사항
// - 범용 유형이어야 한다
// - 생성자가 필요하다
// - 변환 함수가 필요하다
// - 주어진 모든 스트림의 모든 메시지를 받아 하나씩 내보내는 join 함수가 필요하다
// - 모나드 규칙을 따라야 한다.

// 다음과 같은 작업을 진행해 반응형 스트림을 모나드로 만들자
// - 스트림 변환 액터 만들기
// - 주어진 값의 스트림을 생성하는 액터 만들기
// - 한 번에 여러 스트림을 listen해 이들로부터 오는 메시지를 내보낼 수 있는 액터 만들기

// 메시지를 받기 위한 싱크 만들기
// 테스트용으로 메시지를 수신하지만 메시지를 보내지 않는 액터인 싱크 객체를 구현해보자
// 싱크 객체의 구현
namespace detail {
  template <typename Sender,
            typename Function,
            typename MessageType = typename Sender::valuet_type>
  class sink_impl {
public:
  sink_impl(Sender&& sender, Function function)
    : m_sender(std::move(sender))
    , m_function(function) {
      // 싱크가 생성되면 할당된 발신자에게 자동으로 연결된다.
      m_sender.set_message_handler(
        [this](MessageType&& message) {
          process_message(
              std::move(message));
        }
      );
    }

  void process_message(MessageType&& message) const {
    // 메시지를 받으면 사용자가 정의한 함수에 메시지를 전달한다.
    std::invoke(m_function, std::move(message));
  }

private:
  Sender m_sender;
  Function m_function;
  };
}

// 이제 sink_impl의 인스턴스를 만드는 functor가 필요하다
template <typename Sender, typename Function>
auto sink(Sender&& sender, Function&& function) {
  return detail::sink_impl<Sender, Function> (
      std::forward<Sender>(sender),
      std::forward<Function>(function));
}

// 이를 사용하면
// 서비스 시작하기
int main(int argc, char* argv[]) {
  // io_service는 이벤트 루프를 처리하는 Boost.Asio의 클래스다.
  // 이것은 이벤트를 listen해 이벤트에 맞는 적절한 콜백 람다를 호출한다
  boost::asio::io_service event_loop;

  // 서비스를 만들어 싱크에 연결한다
  auto pipeline =
    sink(sevice(event_loop),
        [](const auto& message) {
        std::cerr << message << std::endl;
        });
  // 이벤트 처리를 시작한다.
  event_loop.run();
}

// 이를 파이프 기반 표기법으로 변경해보자
namespace detail {
  template <typename Function>
    struct sink_helper {
      Function function;
    };
}

template <typename Sender, typename Function>
auto operator| (Sender&& sender, detail::sink_helper<Function> sink) {
  return detail::sink_impl<Sender, Function>(std::forward<Sender>(sender), sink.function);
}

// 이를 사용하면
auto sink_to_cerr =
  sink([](const auto& message) {
      std::cerr << message << std::endl;
      });

auto pipeline = service(event_loop) | sink_to_cerr;

// 반응형 스트림 변환
// 반응형 스트림을 모나드로 만드는 작업 중 가장 중요한 작업은 transform 스트림 modifier를 만드는 것이다.
// transform변경자는 메시지를 수신하고 수신과 송신 둘다를 수행하는 적절한 엑터가 될 수 있다.
// Transform 스트림 변경자 구현
namespace detail {
  template<
    typename Sender,
    typename Transformation,
    typename SourceMessageType=
      typename Sender::value_type,
    typename MessageType =
      decltype(std::declval<Transformation>()(
            std::declval<SourceMessageType>()))>
  class transform_impl {
  public:
    using value_type = MessageType;

    transform_impl(Sender&& sender, Transformation transformation)
      : m_sender(std::move(sender))
      , m_transformation(transformation) {}

    template <typename EmitFunction>
    void set_message_handler(EmitFunction emit) {
      m_emit = emit;
      m_sender.set_message_handler(
          [this](SourceMessageType&& message) {
          process_message(
              std::move(message));
          });
    }

    void process_message(SourceMessageType&& message) const {
      m_emit(std::invoke(m_transformation, std::move(message)));
    }

  private:
    Sender m_sender;
    Transformation m_transformation;
    std::function<void(MessageType&&)> m_emit;
  }
}

// ex) 메시지를 출력하기 전에 메시지를 잘라내려면 다음과 같이 한다.
auto pipeline = 
  service(event_loop)
  | transform(trim)
  | sink_to_cerr;

// 주어진 값에 대해 스트림 만들기
// 모나드로 만들기 위해서 값에서 스트림을 만드는 방법과 join 함수가 필요하다
// 값이나 값 목록이 있으면 이들 값을 내보내는 스트림을 작성해보자
// 이 스트림은 메세지를 수락하지 않고 이를 내보내는 작업만 수행한다

template <typename T>
class values {
public:
  using value_type = T;

  explicit values(std::initializer_list<T> values) : m_values(values) {}

  template <typename EmitFunction>
  void set_message_handler(EmitFunction emit) {
    m_emit = emit;
    std::for_each(m_values.cbegin(), m_values.cend(),
        [&](T value) { m_emit(std::move(value)); });
  }

private:
  std::vector<T> m_values;
  std::function<void(T&&)> m_emit;
};

// 이 클래스는 반응형 스트림의 모나드 생성자로 사용될 수 있다.
// 값을 sink 객체에 직접 전달해서 이것이 작동하는지의 여부를 쉽게 테스트할 수 있다.

auto pipeline = values{42} | sink_to_cerr;

// 스트림의 스트림을 조인
// 이제 join함수를 만들어보자
// 다음과 같이 할 수 있기를 원할 것이다.
auto pipeline =
  values{42042, 42043, 42044}
  | transform([&](int port) {
      return service(event_loop, port);
   })
  | join()
  | sink_to_cerr;

// join 변환 구현 (transform과 유사하지만 join은 새 스트림인 메시지를 수신한다는 것이 다르다)
namespace detail{
  template <
    typename Sender,
    // 리슨할 필요가 있는 스트림의 유형
    typename SourceMessageType =
      typename Sender::value_type,
    // 리슨 중인 스트림에서 보낸 메시지의 유형
    // (리스너로 전달해야 하는 메시지)
    typename MessageType =
      typename SourceMessageType::value_type>

  class join_impl {
    public:
      using value_type = MessageType;
      
      ...

      void process_message(SourceMessageType&& source) {
        // 리슨해야 할 새 스트림을 얻으면 이를 저장하고
        // 우리의 것으로 메시지를 전달한다.
        m_source.emplace_back(std::move(source));
        m_source.back().set_message_handler(m_emit);
      }

    private:
      Sender m_sender;
      std::function<void(MessageType&&)> m_emit;
      // 리슨하는 모든 스트림을 저장해 이들의 수명을 연장한다.
      // 재할당 횟수를 최소화하기 위해 리스트를 사용한다.
      std::list<SourceMessageType> m_sources;
  };
}

// 반응형 스트림 필터링
// filter를 만들어보자
// 스트림에서 메시지를 필터링하는 변환 액터
template <typename Sender,
  typename Predicate,
  // 보내는 메시지와 동일한 유형의 메시지를 받는다.
  typename MessageType =
    typename Sender::value_type>
class filter_impl {
public:
  using value_type = MessageType;

  ...

  void prcess_message(MessageType&& message) const {
    if(std::invoke(m_predicate, message)) {
      m_emit(std::move(message));
    }
  }

private:
  Sender m_sender;
  Predicate m_predicate;
  std::function<void(MessageType&&)> m_emit;
};

// 다음과 같은 사용이 가능하다
auto pipeline =
  service(event_loop)
  | transform(trim)
  | filter([](const std::string& message) {
    return message.length() > 0 &&
    message[0] != '#';
    })
  | sink_to_cerr;

// 반응형 스트림에서의 오류처리
// 문자열을 JSON 객체로 파싱하기 expected<T,E>를 사용하자
auto pipeline =
  service(event_loop)
  | transform(trim)
  | filter([](const std::string& message) {
      return message.length() > 0 &&
        message[0] != '#';
    })
  | transform([](const std::string& message) {
    return mtry([&] {
        return json::parse(message);
      });
    })
  | sink_to_cerr;

// 북마크의 URL과 텍스트를 보관하는 구조체를 정의하고 JSON 객체를 받아서 이 객체에 필요한 데이터가 있다면 북마크를 제공하고 그렇지 않다면 오류를 제공하는 함수를 만든다.
struct bookmark_t {
  std::string url;
  std::string text;
};

using expected_bookmark = expected<bookmark_t, std::exception_ptr>;

exepcted_bookmark bookmark_from_json(const json& data) {
  return mtry([&]{
    return bookmark_t{data.at("FirstURL"), data.at("Text")};  
  })
}

// 유효한 값 만을 사용해야 하므로
auto pipeline = 
  service(event_loop)
  | transform(trim)
  | filter(...)

  // 유효한 JSON 객체만을 구한다.
  | transform([](const std::string& message) {
      return mtry([&] {
        return json::parse(message);
      });
    })
  | filter(&expected_json::is_valid)
  | transform(&expected_json::get)

  //유효한 북만크만을 구한다.
  | transform(bookmark_from_json)
  | filter(&expected_bookmark::is_valid)
  | transform(&expected_bookmark::get)
  | sink_to_cerr;

// 이방식은 에러를 잃어버리고 장황하다. 모나드를 써서 장황함을 없애보자.
// expected를 모나드로 처리하기
auto pipeline = 
  service(event_loop)
  | transform(trim)
  | filter(...)

  // 이지점까지는 정상 값의 스트림을 가진다.
  // 여기서 expected 인스턴스의 스트림을 얻게 된다.
  // 즉, 모내드 내의 모나드다.
  | transform([](const std::string& message) {
      return mtry([&] {
        return json::parse(message);
      });
    })

  // expected 인스턴스를 변환하기 위해 mbind를 사용할 수 있다면 expected 객체의 스트림에 대해 작업을 하기 위해 transform을 사용해 이를 리프팅시킬 수 있다.
  | transform([](const auto& exp_json) {
      return mbind(exp_json, bookmark_from_json);
    })

  // 원하는 만큼의 많은 수의 변환(오류가 발생할 수 있는)을 연결할 수 있다.

  ...

  | sink_to_cerr;

// 클라이언트에 응답
// 메시지와 같이 소켓을 보유하는 구조체
template <typename MessageType>
struct with_client {
  MessageType value;
  tcp::socket* socket;

  void reply(const std::string& message) const {
    //async_write가 비동기 도장을 마칠 때까지
    //메시지를 복사해 보관한다.
    auto sptr = std::make_shared<std::string>(message);
    boost::asio::async_write(
      *socket,
      boost::asio::buffer(*sptr, sptr->length()),
      [sptr](auto, auto){});
  }

};

//일반 문자열 대신에 with_client<std::string>을 사용
auto pipeline =
  service(event_loop)
  ...
  | sink([](const auto& message) {
      const auto exp_bookmark = message.value;

      if(!exp_bookmark) {
        message.reply("ERROR: Request not understood\n");
        return;
      }

      if(exp_bookmark->text.find("C++") != std::string::npos) {
        message.reply("OK: " + to_string(exp_bookmark.get()) + "\n");
      } else {
        message.reply("ERROR: NOT a C++ related link\n");
      }
    });

//위의 내용도 mbind를 이용할 수 있다. 
//reactive::operators 네임스페이스에 존재하는 반응형 스트림 용도로 구현된 transform과 filter를 with_client 값의 반응형 스트림에 동작하도록 다음과 같이 재정의하기를 원한다.

auto transform = [](auto f) {
  return reactive::operators::transform(lift_with_client(f));
};

auto filter = [](auto f) {
  return reactive::operators::filter(apply_with_client(f));
};

// lift_with_client는 T1에서 T2로의 모든 함수를 with_client<T1>에서 with_client<T2>로의 함수로 리프트하는 간단한 함수다.
// apply_with_client는 비슷한 일을 하지만 래핑되지 않은 결과 값을 with_client 객체에 두는 대신 반환한다.

// 이를 이용하면 변경없이 서버의 최종 버전이 완성된다. example:bookmark-service-with-reply/main.cpp에서 이용 가능하다.
// 서버의 최종 버전
auto transform = [](auto f) {
  return reactive::operators::transform(lift_with_client(f));
};
auto filter = [](auto f) {
  return reactive::operators::filter(apply_with_client(f)):
};

boost::asio::io_service event_loop;

auto pipeline =
  service(event_loop)
  | transform(trim)

  | filter([](const std::string& message) {
      return message.length() > 0 && message[0] != '#';
    })

  | transform([](const std::string& message) {
      return mtry([&]{ return json::parse(message); });
    })

  | transform([](const auto& exp) {
      return mbind(exp, bookmark_from_json);
    })

  | sink([](const auto& message) {
      const auto exp_bookmark = message.value;
      if(!exp_bookmark) {
        message.reply("ERROR: Request not understood\n");
        return;
      }

      if(exp_bookmark->text.find("C++") != std::string::npos) {
        message.reply("OK: " + to_string(exp_bookmark.get()) + "\n");
      } else {
        message.reply("ERROR: Not a C++-related link\n");
      }
    });

std::cerr << "Service is running... \n";
event_loop.run();

// 가변 상태로 액터 만들기
// 이미 join 로직에서 가변 상태가 사용됨. 단일 스레드에서 돌고 동기화가 필요없다면 가변상태여도 괜찮다.

// 액터로 분산 시스템 작성
// 액터는 독립적이므로 이미 분산시스템이다. 문제는 메시지 전달 시스템으로 멀티스레드 실행인 경우 각 스레드에 메시지 처리 루프를 만들고 올바른 메시지를 적절한 루프에 전달해 올바른 액터로 전달하는 방법을 알아야 한다.
