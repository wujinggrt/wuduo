#include <cassert>
#include <iostream>
#include <cstring>
#include <utility>

#include <sys/socket.h>
#include <unistd.h>

#include "acceptor.h"
#include "channel.h"
#include "log.h"
#include "util.h"

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
    int err = errno;
    LOG_ERROR("setsockopt SO_REUSEADDR, [%d:%s]", err, strerror_thread_local(err));
  }
  if (::setsockopt(acceptfd_, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == -1) {
    int err = errno;
    LOG_ERROR("setsockopt SO_REUSEPORT, [%d:%s]", err, strerror_thread_local(err));
  }
  if (::bind(acceptfd_, local.address(), local.length()) == -1) {
    int err = errno;
    LOG_ERROR("failed to bind on sockfd[%d], [%d:%s]", acceptfd_, err, strerror_thread_local(err));
  }
}

Acceptor::~Acceptor() {
  ::close(acceptfd_);
}

void Acceptor::listen() {
  loop_->assert_in_loop_thread();
  assert(!listening_);
  listening_ = true;
  if (::listen(acceptfd_, SOMAXCONN) == -1) {
    LOG_FATAL("listen()");
  }
  channel_.set_read_callback([this] {
    on_incoming_connection();
  });
  channel_.enable_reading();
  LOG_INFO("sockfd[%d] accept4 enabled reading", acceptfd_);
}

void Acceptor::on_incoming_connection() {
  loop_->assert_in_loop_thread();
  sockaddr_in peer;
  std::memset(&peer, 0, sizeof(peer));
  socklen_t len = static_cast<socklen_t>(sizeof(peer));
  for (;;) {
    int connect_fd = ::accept4(acceptfd_, reinterpret_cast<sockaddr*>(&peer), 
        reinterpret_cast<socklen_t*>(&len), SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (connect_fd == -1) {
      int saved_errno = errno;
      if ((saved_errno == EAGAIN) || (saved_errno == EWOULDBLOCK)) {
        return ;
      }
      LOG_FATAL("accept4(): %s(fd:%d, peer*:%p, len:%p[=%d])", 
          strerror_thread_local(saved_errno), acceptfd_, reinterpret_cast<sockaddr*>(&peer), &len, len);
#if 0
      if ((saved_errno != ECONNABORTED) | (saved_errno != EMFILE)) {
        LOG_FATAL("unexcepted accept4");
      }
#endif
      return ;
    }
    if (new_connection_callback_) {
      InetAddress inet_peer{peer};
      new_connection_callback_(connect_fd, inet_peer);
    } else {
      ::close(connect_fd);
    }
  }

}

}
