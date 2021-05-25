#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>

#include <sys/socket.h>

#include "../wuduo/tcp_server.h"
#include "../wuduo/event_loop.h"
#include "../wuduo/inet_address.h"
#include "../wuduo/tcp_connection.h"
#include "../wuduo/log.h"
#include "../wuduo/util.h"
#include "../wuduo/http_data.h"

using namespace wuduo;

void on_message(const TcpConnectionPtr& conn, std::string msg) {
  int connection_fd = conn->get_channel()->get_fd();
  LOG_INFO("connection_fd[%d] read[%s]", connection_fd, msg.c_str());
  InetAddress local = InetAddress::local_from(connection_fd);
  InetAddress peer = InetAddress::peer_from(connection_fd);
  std::string echo_msg{"Hello, server"};
  std::string local_str = std::string{"("} + local.get_ip() + ", " + std::to_string(local.get_port()) + ")";
  std::string peer_str = std::string{", client("} + peer.get_ip() + ", " + std::to_string(peer.get_port()) + ")";
  echo_msg += local_str + peer_str;
  echo_msg += " - " + msg;
  if (::write(connection_fd, echo_msg.data(), echo_msg.size()) == -1) {
    LOG_ERROR("connection_fd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
  }
}

void on_http_message(const TcpConnectionPtr& conn, std::string msg) {
  int connection_fd = conn->get_channel()->get_fd();
  LOG_INFO("Request message(from[%d]):\n%s", connection_fd, msg.c_str());
  LOG_INFO("Respond message:\n%s", kHelloWorld);
  auto request_line = RequestLine::from(msg);
  LOG_INFO("Parsed request line[method:%s url:%s version:%s]", request_line->method.c_str(), request_line->url.c_str(), request_line->version.c_str());
  if (::write(connection_fd, kHelloWorld, std::strlen(kHelloWorld)) == -1) {
    LOG_ERROR("connection_fd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
  }
  LOG_INFO("sockfd[%d] - Message responded", connection_fd);
}

int main() {
  wuduo::EventLoop loop;
  wuduo::InetAddress address{12000};
  wuduo::TcpServer server{&loop, address};
  //server.set_message_callback(on_message);
  server.set_message_callback(on_http_message);
  server.start();
  loop.loop();
}
