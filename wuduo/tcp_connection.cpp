#include <utility>

#include <unistd.h>

#include "tcp_connection.h"

namespace wuduo {

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, InetAddress peer)
  : loop_{loop},
  channel_{loop, sockfd},
  peer_{std::move(peer)},
  reading_{false} {}

TcpConnection::~TcpConnection() {
  ::close(channel_.get_fd());
}

}
