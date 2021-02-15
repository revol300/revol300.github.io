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
};
