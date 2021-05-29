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
#include "../wuduo/http/http_server.h"

using namespace wuduo;
using namespace wuduo::http;

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

int main() {
  EventLoop loop;
  InetAddress address{12000};
  HttpServer server{&loop, address};
  server.start();
  loop.loop();
}
