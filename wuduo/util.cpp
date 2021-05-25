#include <string.h>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "util.h"
#include "log.h"

namespace wuduo {

void set_nodelay(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

void set_keep_alive(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

int get_socket_error(int sockfd) {
  int optval;
  socklen_t len = static_cast<socklen_t>(sizeof(optval));
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &len) == -1) {
    LOG_ERROR("getsockopt() failed to run, maybe bad sockfd[%d], [%d:%s]", sockfd, errno, strerror(errno));
    return errno;
  }
  return optval;
}

thread_local char t_errnobuf[512];

const char* strerror_thread_local(int saved_errno) {
  return ::strerror_r(saved_errno, t_errnobuf, sizeof(t_errnobuf));
}

}
