#pragma once

#include <functional>
#include <utility>

#include "noncopyable.h"
#include "event_loop.h"
#include "inet_address.h"
#include "acceptor.h"
#include "event_loop_thread_pool.h"

namespace wuduo {

class TcpServer : noncopyable {
 public:
  TcpServer(EventLoop* loop, InetAddress local);
  ~TcpServer();

  void set_new_connection_callback(NewConnectionCallback cb) {
    new_connection_callback_ = std::move(cb);
  }

  void start();

 private:
  EventLoop* loop_;
  InetAddress local_;
  Acceptor acceptor_;
  EventLoopThreadPool event_loop_thread_pool_;
  NewConnectionCallback new_connection_callback_;
};

}
