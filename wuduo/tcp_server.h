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
#include "callbacks.h"

namespace wuduo {

class TcpConnection;

class TcpServer : noncopyable {
 public:
  TcpServer(EventLoop* loop, InetAddress local);
  ~TcpServer();

  void set_connection_callback(ConnectionCallback cb) {
    connection_callback_ = std::move(cb);
  }
  void set_message_callback(MessageCallback cb) {
    message_callback_ = std::move(cb);
  }

  // thread safe, can be called by other thread but only once.
  void start();

 private:
  void new_connection(int connect_fd, InetAddress peer);

  // this can be called directly to force tcp conn to close.
  void remove_connection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  InetAddress local_;
  Acceptor acceptor_;
  EventLoopThreadPool event_loop_thread_pool_;
  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;

  std::unordered_set<std::shared_ptr<TcpConnection>> connections_;
};

}
