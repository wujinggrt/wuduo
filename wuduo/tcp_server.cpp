#include <string>

#include <sys/socket.h>
#include <netinet//in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <endian.h>
#include <unistd.h>

#include "util.h"
#include "tcp_server.h"
#include "log.h"
#include "event_loop_thread.h"

namespace wuduo {

constexpr const int kNumThreads = 1;

TcpServer::TcpServer(EventLoop* loop, InetAddress local)
  : loop_{loop},
  local_{local},
  acceptor_{loop, local},
  event_loop_thread_pool_{loop, kNumThreads} {
  ::signal(SIGPIPE, SIG_IGN);
  new_connection_callback_ = [this] (int connect_fd, InetAddress peer) {
    const auto* sock_addr = reinterpret_cast<const sockaddr_in*>(peer.get_address());

    LOG_INFO("New connection from (%s, %d)", ::inet_ntoa(sock_addr->sin_addr), sock_addr->sin_port);
    std::cerr << std::endl;

    set_nodelay(connect_fd);
    set_keep_alive(connect_fd);

    auto* io_loop = event_loop_thread_pool_.get_next_loop();
    io_loop->run_in_loop([connect_fd, peer] {
      std::string echo_msg{"Hello, "};
      echo_msg += inet_ntoa(reinterpret_cast<const sockaddr_in*>(peer.get_address())->sin_addr);
      ::write(connect_fd, echo_msg.c_str(), echo_msg.size());
      ::close(connect_fd);
    });
  };
}

TcpServer::~TcpServer() {
  LOG_INFO("~TcpServer");
}

void TcpServer::start() {
  loop_->run_in_loop([this] {
    event_loop_thread_pool_.start();
    acceptor_.set_new_connection_callback(new_connection_callback_);
    acceptor_.listen();
  });
}

}
