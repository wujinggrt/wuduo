#include <utility>
#include <cstring>

#include <unistd.h>

#include "tcp_connection.h"
#include "log.h"
#include "util.h"

namespace wuduo {

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, InetAddress peer)
  : loop_{loop},
  channel_{loop, sockfd},
  peer_{std::move(peer)},
  reading_{false} {
  channel_.set_read_callback([this] {
      handle_read();
  });
  set_nodelay(sockfd, true);
  set_keep_alive(sockfd, true);
}

TcpConnection::~TcpConnection() {
  ::close(channel_.get_fd());
}

void TcpConnection::established() {
  channel_.enable_reading();
}

void TcpConnection::handle_read() {
  loop_->assert_in_loop_thread();
  auto fd = channel_.get_fd();
  char buf[1024] = "";
  auto num_read = ::read(fd, buf, sizeof(buf));
  if (num_read > 0) {
    if (message_callback_) {
      message_callback_(shared_from_this(), std::string{buf});
    }
  } else if (num_read == 0) {
    handle_close();
  } else {
    LOG_ERROR("TcpConnection::handle_read(), ::read() [%s]", std::strerror(errno));
    handle_error();
  }
}

void TcpConnection::handle_close() {
  channel_.disable_all();
  if (close_callback_) {
    close_callback_(shared_from_this());
  }
}

void TcpConnection::handle_error() {
  int err = get_socket_error(channel_.get_fd());
  LOG_ERROR("handle_error(), SO_ERROR=%d [%s]", err, std::strerror(err));
}

}
