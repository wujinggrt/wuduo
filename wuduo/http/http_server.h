#pragma once

#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <mutex>
#include <optional>

#include "wuduo/http/http_request.h"
#include "wuduo/noncopyable.h"
#include "wuduo/inet_address.h"
#include "wuduo/event_loop.h"
#include "wuduo/tcp_server.h"
#include "wuduo/callbacks.h"

namespace wuduo::http {

[[maybe_unused]] constexpr const std::string_view kHelloWorld =
  "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8 \r\n\r\nHello wOrld!";

[[maybe_unused]] constexpr const std::string_view kBadRequest =
  "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html;charset=utf-8 \r\n\r\n";

struct HttpConnectionMetadata {
  enum class ParsingPhase { kRequestLine, kHeaderLines, kEntityBody, kFinished };
  enum class ParsingError { kCorrect, kRequestLine, kHeaderLine, kEntityBody, kUnknown };


  ParsingPhase parsing_phase{ParsingPhase::kRequestLine};
  ParsingError parsing_error{ParsingError::kCorrect};
  std::string in_buffer;
  std::string out_buffer;
  RequestLine request_line;
};

class HttpServer : noncopyable {
 public:
  HttpServer(EventLoop* loop, InetAddress address);

  void start();

 private:
  void on_message(const TcpConnectionPtr& conn, std::string msg);

  void handle_error(const TcpConnectionPtr& conn, int error_code, std::string_view short_msg);

  void send_error_page(const TcpConnectionPtr& conn, int error_code, std::string_view short_msg) const;

  TcpServer server_;
};

}
