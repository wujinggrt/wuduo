#include <cassert>
#include <cstring>
#include <utility>
#include <tuple>

#include <unistd.h>
#include <sys/uio.h>

#include "wuduo/http/http_server.h"
#include "wuduo/http/http_request.h"
#include "wuduo/http/http_context.h"
#include "wuduo/http/http_response.h"
#include "wuduo/tcp_connection.h"
#include "wuduo/log.h"
#include "wuduo/util.h"

namespace wuduo::http {

HttpServer::HttpServer(EventLoop* loop, InetAddress address)
  : server_{loop, address}
{
  server_.set_connection_callback([] (const TcpConnectionPtr& conn) {
    if (conn->is_connected()) {
      conn->set_context(std::make_any<HttpContext>());
      LOG_DEBUG("setting context completed");
    }
  });
  server_.set_message_callback([this] (const auto& conn, Buffer* buf) {
    on_message(conn, buf);
  });
}

void HttpServer::start() {
  server_.start();
}

void HttpServer::on_message(const TcpConnectionPtr& conn, Buffer* buf) {
  // can be called only in io_loop of the corresponding conn.
  // EOF, peer closed.
  HttpContext* context = std::any_cast<HttpContext>(conn->context());

  auto num_read = buf->readable_bytes();
  std::string msg{buf->peek(), num_read};
  LOG_INFO("sockfd[%d] num_read[%d], contents:\n%s", conn->channel()->get_fd(), num_read, msg.c_str());

  if (!context->parse_request(buf)) {
    HttpResponse response;
    response.set_status_code(StatusCode::k400BadRequest);
    response.set_error_page_to_entity_body();
    Buffer buf;
    response.append_to(&buf);
    // conn->send(buf.peek(), buf.readable_bytes());
    conn->send(&buf);
    conn->shutdown();
    //handle_error(conn, 400, "Bad request");
  }

  if (context->parsing_completed()) {
    auto* request = context->request();
    HttpResponse response{request->is_close_connection()};
    response.set_status_code(StatusCode::k200Ok);
    // response.set_entity_body(std::string{kHelloWorld});
    response.analyse(request);
    Buffer buf;
    response.append_to(&buf);
    conn->send(buf.peek(), buf.readable_bytes());
    if (response.close_connection()) {
      // should be shutdown
      conn->shutdown();
    }
    context->reset();
  }
}

void HttpServer::handle_error(const TcpConnectionPtr& conn, int error_code, std::string_view short_msg) {
  send_error_page(conn, error_code, short_msg);
  conn->force_close();
}

void HttpServer::send_error_page(const TcpConnectionPtr& conn, int error_code, std::string_view short_msg) const {
  std::string status = "HTTP/1.1 " + std::to_string(error_code) + " " + std::string{short_msg} + "\r\n";
  std::string_view headers{"Content-Type: text/html;charset=utf-8 \r\n\r\n"};
  std::string_view body = "<html><title>出错了~</title>";
  std::string error_contents = "<body><h1>ERROR<hr /></h1><p>" + 
    std::to_string(error_code) + ", " + std::string{short_msg} + "</p></body></html>";
  constexpr const int kNumIov{4};
  iovec iov[kNumIov];
  iov[0].iov_base = status.data();
  iov[0].iov_len = status.size();
  iov[1].iov_base = const_cast<void*>(reinterpret_cast<const void*>(headers.data()));
  iov[1].iov_len = headers.size();
  iov[2].iov_base = const_cast<void*>(reinterpret_cast<const void*>(body.data()));
  iov[2].iov_len = body.size();
  iov[3].iov_base = error_contents.data();
  iov[3].iov_len = error_contents.size();
  int connection_fd = conn->channel()->get_fd();
  LOG_INFO("sockfd[%d] - Message responding...", connection_fd);
  if (::writev(connection_fd, iov, kNumIov) == -1) {
    LOG_ERROR("sockfd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
  }
    LOG_INFO("sockfd[%d] - Message responded", connection_fd);
}

}
