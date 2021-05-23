#pragma once

#include <cstdint>
#include <string_view>

#include <netinet/in.h>

namespace wuduo {

class InetAddress {
 public:
  InetAddress(uint16_t port, bool loopback = false);
  InetAddress(uint16_t port, std::string_view ip_address);
  InetAddress(const sockaddr_in& address);

  void set_address(const sockaddr_in& address) {
    address_ = address;
  }
  const sockaddr* get_address() const {
    return reinterpret_cast<const sockaddr*>(&address_);
  };
  socklen_t get_len() const {
    return sizeof(address_);
  }

 private:
  sockaddr_in address_;

};

}
