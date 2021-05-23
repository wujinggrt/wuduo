#include <cstring>

#include <endian.h>
#include <arpa/inet.h>

#include "inet_address.h"

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

}
