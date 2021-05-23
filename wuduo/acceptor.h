#pragma once

#include "inet_address.h"
#include "event_loop.h"
#include "channel.h"

namespace wuduo {

// the user should mannualy close the socket that connected the peer client.
  using NewConnectionCallback = 
    std::function<void(int connect_fd, InetAddress peer)>;

class Acceptor {
 public:

  Acceptor(EventLoop* loop, InetAddress local);
  ~Acceptor();

  void listen();
  bool is_listening() const { return listening_; }

  void set_new_connection_callback(NewConnectionCallback cb) {
    new_connection_callback_ = cb;
  }

 private:
  EventLoop* loop_;
  const int acceptfd_;
  bool listening_;
  Channel channel_;
  InetAddress local_;
  NewConnectionCallback new_connection_callback_;
};

}
