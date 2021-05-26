#pragma once

#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <mutex>
#include <optional>

#include "noncopyable.h"
#include "inet_address.h"
#include "event_loop.h"
#include "tcp_server.h"
#include "callbacks.h"

namespace wuduo {

[[maybe_unused]] constexpr const std::string_view kHelloWorld =
  "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8 \r\n\r\nHello wOrld!";

[[maybe_unused]] constexpr const std::string_view kBadRequest =
  "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html;charset=utf-8 \r\n\r\n";

struct RequestLine {
  std::string method;
  std::string url;
  std::string version;

  static std::optional<RequestLine> from(std::string_view line);
};

struct TcpConnectionMetadata {
  enum class ParsingState { kRequestLine, kHeaderLine, kEntityBody, kFinished };

  std::string in_buffer;
  std::string out_buffer;
  ParsingState parsing_state{ParsingState::kRequestLine};
};

class HttpServer : noncopyable {
 public:
  HttpServer(EventLoop* loop, InetAddress address);

  void start();

 private:
  void handle_read(const TcpConnectionPtr& conn, std::string msg);

  EventLoop* loop_;
  InetAddress local_;
  TcpServer server_;
  std::mutex mutex_;
  std::unordered_map<TcpConnectionPtr, TcpConnectionMetadata> connection_metadata_;
};

}
