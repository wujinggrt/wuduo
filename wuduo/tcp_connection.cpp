#include <utility>
#include <cassert>
#include <cstring>

#include <unistd.h>

#include "tcp_connection.h"
#include "log.h"
#include "util.h"

namespace wuduo {

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, InetAddress peer)
  : loop_{loop},
  state_{kConnecting},
  channel_{loop, sockfd},
  peer_{std::move(peer)},
  reading_{false} {
  channel_.set_read_callback([this] { handle_read(); });
  channel_.set_write_callback([this] { handle_write(); });
  set_nodelay(sockfd, true);
  set_keep_alive(sockfd, true);
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG("TcpConnection dtor sockfd[%d]", channel_.get_fd());
  ::close(channel_.get_fd());
}

void TcpConnection::established() {
  loop_->assert_in_loop_thread();
  assert(state_ == kConnecting);
  set_state(kConnected);
  channel_.enable_reading();

  if (connection_callback_) {
    connection_callback_(shared_from_this());
  }
  LOG_INFO("sockfd[%d] established", channel_.get_fd());
}

// called via tcp server.
void TcpConnection::destroyed() {
  loop_->assert_in_loop_thread();
  if (state_ == kConnected) {
    set_state(kDisconnected);
    if (!channel_.has_none_events()) {
      channel_.disable_all();
    }
    if (connection_callback_) {
      connection_callback_(shared_from_this());
    }
  }
}

void TcpConnection::handle_read() {
  loop_->assert_in_loop_thread();
  auto fd = channel_.get_fd();
  // if the epoller use edge-triggered, read should called until return -1
  int saved_errno = 0;
  auto num_read = in_buffer_.read_fd(fd, &saved_errno);
  if (num_read > 0) {
    if (message_callback_) {
      message_callback_(shared_from_this(), &in_buffer_);
    }
  } else if (num_read == 0) {
    set_state(kDisconnecting);
    LOG_INFO("sockfd[%d] Peer closed", channel_.get_fd());
    handle_close();
    return ;
  } else if (num_read == -1) {
    // ends here.
    if ((saved_errno != EAGAIN) && (saved_errno != EWOULDBLOCK)) {
      LOG_ERROR("::read() [%d:%s]", saved_errno, strerror_thread_local(saved_errno));
      handle_error();
    }
  }
}

void TcpConnection::handle_write() {
  loop_->assert_in_loop_thread();
}

void TcpConnection::handle_close() {
  LOG_DEBUG("sockfd[%d] tcp_connection state_[%s]", channel_.get_fd(), get_state_string().c_str());
  assert((state_ == kConnected) || (state_ == kDisconnecting));
  set_state(kDisconnected);
  // the epoller will not monitor this sockfd any longer,
  // the handle_read will not be called after this.
  if (!channel_.has_none_events()) {
    channel_.disable_all();
  }
  auto guard_this_from_releasing = shared_from_this();
  if (connection_callback_) {
    connection_callback_(guard_this_from_releasing);
  }
  // may cause dtor calling.
  if (close_callback_) {
    // may return immediately, performed by lambda in loop.
    close_callback_(guard_this_from_releasing);
  }
}

void TcpConnection::handle_error() {
  int err = get_socket_error(channel_.get_fd());
  LOG_ERROR("handle_error(), SO_ERROR=%d [%s]", err, std::strerror(err));
}

void TcpConnection::force_close() {
  if (state_ == kConnected || state_ == kDisconnecting) {
    set_state(kDisconnecting);
    loop_->run_in_loop([this] {
      if (state_ == kConnected || state_ == kDisconnecting) {
        handle_close();
      }
    });
}
}

}
