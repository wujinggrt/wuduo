#include <string>
#include <utility>

#include <sys/socket.h>
#include <netinet//in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <endian.h>
#include <unistd.h>

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
  auto connection = std::make_shared<TcpConnection>(io_loop, connection_fd, peer);
  connections_.insert(connection);
  connection->set_message_callback([connection_fd](const TcpConnectionPtr&, std::string msg) {
    LOG_INFO("Readed msg [%s], \nechoing...", msg.c_str());

    InetAddress local = InetAddress::local_from(connection_fd);
    InetAddress peer = InetAddress::peer_from(connection_fd);
    std::string echo_msg{"Hello, server"};
    std::string local_str = std::string{"("} + local.get_ip() + ", " + std::to_string(local.get_port()) + ")";
    std::string peer_str = std::string{", client("} + peer.get_ip() + ", " + std::to_string(peer.get_port()) + ")\n";
    echo_msg += local_str + peer_str;
    echo_msg += "\n" + msg;
    if (::write(connection_fd, echo_msg.c_str(), echo_msg.size()) < 0) {
      int err = get_socket_error(connection_fd);
      LOG_ERROR("::write(sockfd, ), [%d:%s]", err, strerror_thread_local(err));
    }
  });
  connection->set_close_connection_callback([this, connection_fd](const TcpConnectionPtr& ptr) {
    ::close(connection_fd);
    connections_.erase(ptr);
    LOG_INFO("Closed connection_fd[%d]", connection_fd);
  });
  io_loop->run_in_loop([connection, connection_fd] {
    connection->established();
    LOG_INFO("New connection established, connection_fd[%d]", connection_fd);
  });
}

}
