#include <string>
#include <utility>

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

[[maybe_unused]]constexpr const int kNumThreads = 1;

TcpServer::TcpServer(EventLoop* loop, InetAddress local)
  : loop_{loop},
  local_{local},
  acceptor_{loop, local},
  event_loop_thread_pool_{loop, kNumThreads} {
  ::signal(SIGPIPE, SIG_IGN);
  acceptor_.set_new_connection_callback([this] (int connect_fd, InetAddress peer) {
    handle_new_connection(connect_fd, std::move(peer));
  });
}

TcpServer::~TcpServer() {
  LOG_INFO("~TcpServer");
}

void TcpServer::start() {
#if 0
  event_loop_thread_pool_.start();
#endif

#if 1
  // avoid the other thread calling this, make it thread safe.
  loop_->run_in_loop([this] {
    acceptor_.listen();
  });
#endif

#if 0
  loop_->run_in_loop([this] {
    event_loop_thread_pool_.start();
    acceptor_.set_new_connection_callback([this] (int connect_fd, InetAddress peer) {
      handle_new_connection(connect_fd, std::move(peer));
    });
    acceptor_.listen();
  });
#endif
}

void TcpServer::handle_new_connection(int connect_fd, InetAddress peer) {
  LOG_INFO("New connection, local (%s, %d), peer (%s, %d)",
      local_.get_ip().c_str(), local_.get_port(),
      peer.get_ip().c_str(), peer.get_port());

  set_nodelay(connect_fd);
  set_keep_alive(connect_fd);

#if 0
  auto* io_loop = event_loop_thread_pool_.get_next_loop();

  io_loop->run_in_loop([connect_fd, peer] {
#endif
    InetAddress local = InetAddress::local_from(connect_fd);
    InetAddress another_peer = InetAddress::peer_from(connect_fd);
    std::string echo_msg{"Hello, "};
    std::string local_str = std::string{"("} + local.get_ip() + ", " + std::to_string(local.get_port()) + ")";
    std::string another_peer_str = std::string{"("} + another_peer.get_ip() + ", " + std::to_string(another_peer.get_port()) + ")";
    echo_msg += local_str + another_peer_str;
    ::write(connect_fd, echo_msg.c_str(), echo_msg.size());
    ::close(connect_fd);
#if 0
  });
#endif

  if (new_connection_callback_) {
    new_connection_callback_(connect_fd, peer);
  }
}

}
