#pragma once

#include "inet_address.h"
#include "event_loop.h"
#include "channel.h"
#include "callbacks.h"

namespace wuduo {

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
