#include <cstring>
#include <cassert>
#include <string>
#include <utility>
#include <chrono>

#include <sys/socket.h>
#include <netinet//in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <endian.h>
#include <unistd.h>

#include <time.h>

#include "util.h"
#include "tcp_server.h"
#include "log.h"
#include "event_loop_thread.h"
#include "tcp_connection.h"

namespace wuduo {

[[maybe_unused]]constexpr const int kNumThreads = 4;

TcpServer::TcpServer(EventLoop* loop, InetAddress local)
  : loop_{loop},
  local_{local},
  acceptor_{loop, local},
  event_loop_thread_pool_{loop, kNumThreads} {
  ::signal(SIGPIPE, SIG_IGN);
  acceptor_.set_new_connection_callback([this] (int connect_fd, InetAddress peer) {
    new_connection(connect_fd, std::move(peer));
  });
}

TcpServer::~TcpServer() {
  LOG_INFO("~TcpServer");
}

void TcpServer::start() {
  // avoid the other thread calling this, make it thread safe.
  loop_->run_in_loop([this] {
    event_loop_thread_pool_.start();
    acceptor_.listen();
  });
}

void TcpServer::new_connection(int connection_fd, InetAddress peer) {
  LOG_INFO("New connection, fd[%d] local (%s, %d), peer (%s, %d)", connection_fd,
      local_.get_ip().c_str(), local_.get_port(),
      peer.get_ip().c_str(), peer.get_port());
  auto* io_loop = event_loop_thread_pool_.get_next_loop();
  auto conn = std::make_shared<TcpConnection>(io_loop, connection_fd, peer);
  auto [iter, ok] = connections_.insert(conn);
  (void) iter;
  assert(ok);
  conn->set_connection_callback(connection_callback_);
  conn->set_message_callback(message_callback_);
  conn->set_close_callback(
      [this] (const TcpConnectionPtr& conn) { remove_connection(conn); });
  io_loop->run_in_loop([conn] { 
    conn->established();
  });
}

// run in loop, may return immediately.
void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
  loop_->run_in_loop([this, conn] {
  // the bug version as below:
  // loop_->run_in_loop([this, &conn] {
    // here, the connection should be sured not to free because of number of ref count as zero.
    // because we need to call conn->destroyed(), which requires conn is not a dangling ref.
    auto num_erased = connections_.erase(conn);
    (void)num_erased;
    assert(num_erased == 1);
    auto* io_loop = conn->loop();
    io_loop->queue_in_loop([conn] {
      conn->destroyed();
    });
  });
}

}
