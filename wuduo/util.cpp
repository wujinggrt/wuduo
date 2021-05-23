#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "util.h"

namespace wuduo {

void set_nodelay(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

void set_keep_alive(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

}
