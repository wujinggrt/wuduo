#include <utility>
#include <cassert>
#include <cstring>

#include <unistd.h>

#include "tcp_connection.h"
#include "log.h"
#include "util.h"

namespace {

[[maybe_unused]]constexpr const size_t kReadBufferSize = 10;

}

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
}

// called via tcp server.
void TcpConnection::destroyed() {
  loop_->assert_in_loop_thread();
  if (state_ == kConnected) {
    set_state(kDisconnected);
    if (!channel_.has_none_events()) {
      channel_.disable_all();
    }
  }
}

void TcpConnection::handle_read() {
  loop_->assert_in_loop_thread();
  auto fd = channel_.get_fd();
  char buf[kReadBufferSize] = "";
  LOG_DEBUG("sockfd[%d] Handling read", channel_.get_fd());
  // if the epoller use edge-triggered, read should called until return -1
  auto num_read = ::read(fd, buf, sizeof(buf));
  if (num_read >= 0) {
    LOG_DEBUG("sockfd[%d] read num[%d]", channel_.get_fd(), num_read);
    std::string msg{buf, static_cast<std::string::size_type>(num_read)};
    while (num_read == sizeof(buf)) {
      num_read = ::read(fd, buf, sizeof(buf));
      LOG_DEBUG("sockfd[%d] previous buffer is full, this turn reads num[%d]", channel_.get_fd(), num_read);
      if (num_read == -1) {
        int saved_errno = errno;
        if ((saved_errno == EAGAIN) || (saved_errno == EWOULDBLOCK)) {
          LOG_DEBUG("sockfd[%d] currently reads done.", channel_.get_fd(), num_read);
          break;
        } else {
          LOG_ERROR("::read() [%s]", strerror_thread_local(saved_errno));
          handle_error();
          return ;
        }
      } else if (num_read > 0) {
        msg += std::string{buf, static_cast<std::string::size_type>(num_read)};
      } else {
        // == 0
        set_state(kDisconnecting);
        LOG_INFO("sockfd[%d] Peer closed", channel_.get_fd());
        handle_close();
        return ;
      }
    }
    if (message_callback_) {
      message_callback_(shared_from_this(), std::move(msg));
    }
    if (num_read == 0) {
      set_state(kDisconnecting);
      LOG_INFO("sockfd[%d] Peer closed", channel_.get_fd());
      handle_close();
    }
  } else {
    LOG_ERROR("::read() [%s]", std::strerror(errno));
    handle_error();
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
  // must be the last line, it will remove this from tcp server's set.
  if (close_callback_) {
    close_callback_(shared_from_this());
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
