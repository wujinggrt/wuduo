#pragma once

#include <functional>
#include <utility>
#include <unordered_set>
#include <memory>

#include "noncopyable.h"
#include "event_loop.h"
#include "inet_address.h"
#include "acceptor.h"
#include "event_loop_thread_pool.h"

namespace wuduo {

class TcpConnection;

class TcpServer : noncopyable {
 public:
  TcpServer(EventLoop* loop, InetAddress local);
  ~TcpServer();

  void set_new_connection_callback(NewConnectionCallback cb) {
    new_connection_callback_ = std::move(cb);
  }

  // thread safe, can be called by other thread but only once.
  void start();

 private:
  void handle_new_connection(int connect_fd, InetAddress peer);

  EventLoop* loop_;
  InetAddress local_;
  Acceptor acceptor_;
  EventLoopThreadPool event_loop_thread_pool_;
  NewConnectionCallback new_connection_callback_;

  std::unordered_set<std::shared_ptr<TcpConnection>> connections_;
};

}
