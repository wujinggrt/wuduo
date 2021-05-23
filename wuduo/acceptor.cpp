#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <utility>

#include "acceptor.h"
#include "channel.h"
#include "log.h"

namespace {
  int create_socket() {
    int socket_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
    if (socket_fd == -1) {
      std::cerr << "FATAL create_socket, accpetor()\n";
    }
    return socket_fd;
  }
}

namespace wuduo {

Acceptor::Acceptor(EventLoop* loop, InetAddress local)
  : loop_{loop},
  acceptfd_{create_socket()},
  listening_{false},
  channel_{loop, acceptfd_},
  local_{std::move(local)} {
  LOG_INFO("Acceptor::acceptfd_ [%d]", acceptfd_);
  int on = 1;
  if (::setsockopt(acceptfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    std::cerr << "FATAL: setsockopt(), SO_REUSEADDR\n";
  }
  if (::setsockopt(acceptfd_, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == -1) {
    std::cerr << "FATAL: setsockopt(), SO_REUSEPORT\n";
  }
  if (::bind(acceptfd_, local.get_address(), local.get_len()) == -1) {
    std::cerr << ("Failed to bind() on socket_fd\n");
  }
}

Acceptor::~Acceptor() {
  ::close(acceptfd_);
}

void Acceptor::listen() {
  loop_->assert_in_loop_thread();
  listening_ = true;
  if (::listen(acceptfd_, SOMAXCONN) == -1) {
    LOG_FATAL("listen()");
  }
  channel_.set_read_callback([this] {
    loop_->assert_in_loop_thread();
    sockaddr_in peer;
    socklen_t len;
    for (;;) {
      int connect_fd = ::accept4(acceptfd_, reinterpret_cast<sockaddr*>(&peer), &len, SOCK_CLOEXEC | SOCK_NONBLOCK);
      if (connect_fd == -1) {
        LOG_ERROR("accept4() return -1");
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          LOG_INFO("EAGAIN, EWOULDBLOCK");
          return ;
        }
        if ((errno != ECONNABORTED) | (errno != EMFILE)) {
          LOG_FATAL("unexcepted accept4");
        }
        return ;
      }
      LOG_INFO("Connected with peer");
      if (new_connection_callback_) {
        InetAddress inet_peer{peer};
        new_connection_callback_(connect_fd, inet_peer);
      } else {
        ::close(connect_fd);
      }
    }
  });
  channel_.enable_reading();
}

}
