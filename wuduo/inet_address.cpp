#include <cstring>

#include <endian.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#include "util.h"
#include "inet_address.h"
#include "log.h"

namespace wuduo {

InetAddress::InetAddress(uint16_t port, bool loopback) {
  std::memset(&address_, 0, sizeof(address_));
  address_.sin_family = AF_INET;
  address_.sin_port = htobe16(port);
  address_.sin_addr.s_addr = 
    htobe32(loopback ? INADDR_LOOPBACK : INADDR_ANY);
}

InetAddress::InetAddress(uint16_t port, std::string_view ip_address) {
  std::memset(&address_, 0, sizeof(address_));
  address_.sin_family = AF_INET;
  address_.sin_port = htobe16(port);
  uint32_t ip_address_number;
  ::inet_pton(AF_INET, ip_address.data(), &ip_address_number);
  address_.sin_addr.s_addr = htobe32(ip_address_number);
}

InetAddress::InetAddress(const sockaddr_in& address) 
  : address_{address} {}

uint16_t InetAddress::get_port() const {
  return ::be16toh(address_.sin_port);
}

std::string InetAddress::get_ip() const {
  char buf[INET_ADDRSTRLEN] = "";
  ::inet_ntop(AF_INET, &address_.sin_addr, buf, INET_ADDRSTRLEN);
  return std::string{buf};
}

InetAddress InetAddress::local_from(int connect_sockfd) {
  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  socklen_t len = static_cast<socklen_t>(sizeof(addr));
  if (::getsockname(connect_sockfd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
    int err = errno;
    LOG_ERROR("InetAddress::local_from(sockfd: %d), [%d:%s]", connect_sockfd, err, strerror_thread_local(err));
  }
  return InetAddress{*reinterpret_cast<sockaddr_in*>(&addr)};
}

InetAddress InetAddress::peer_from(int connect_sockfd) {
  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  socklen_t len = static_cast<socklen_t>(sizeof(addr));
  if (::getpeername(connect_sockfd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
    int err = errno;
    LOG_ERROR("InetAddress::peer_from(sockfd: %d), [%d:%s]", connect_sockfd, err, strerror_thread_local(err));
  }
  return InetAddress{*reinterpret_cast<sockaddr_in*>(&addr)};
}

}
