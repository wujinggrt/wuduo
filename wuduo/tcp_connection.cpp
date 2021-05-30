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
  LOG_INFO("sockfd[%d] enabled reading", channel_.get_fd());

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

void TcpConnection::send(Buffer* buf) {
  if (state_ != kConnected) {
    return ;
  }
  if (loop_->in_loop_thread()) {
    send_in_loop(buf->peek(), buf->readable_bytes());
    buf->retrieve_all();
    return ;
  }
  loop_->run_in_loop([this, message_will_not_release = buf->retrieve_all_as_string()] {
    send_in_loop(message_will_not_release.data(), message_will_not_release.size());
  });
}

void TcpConnection::send(const char* data, size_t count) {
  if (state_ != kConnected) {
    return ;
  }
  if (loop_->in_loop_thread()) {
    send_in_loop(data, count);
    return ;
  }
  // avoid from buffer has released. just copyed it.
  loop_->run_in_loop([this, saved_from_buffer = std::string{data, count}] {
      send_in_loop(saved_from_buffer.data(), saved_from_buffer.size());
  });
}

void TcpConnection::send_in_loop(const char* data, size_t count) {
  loop_->assert_in_loop_thread();
  if (state_ == kDisconnected) {
    LOG_WARN("disconnected, give up writing");
    return ;
  }
  ssize_t num_wrote = 0;
  // it is ok to write directly now, no pending write.
  if (!channel_.is_writing() && (output_buffer_.readable_bytes() == 0)) {
    num_wrote = ::write(channel_.get_fd(), data, count);
    if (num_wrote >= 0) {
      if (static_cast<size_t>(num_wrote) < count) {
        LOG_DEBUG("write more data");
      }
    } else {
      // error, pending contents to buffer.
      num_wrote = 0;
      if ((errno != EWOULDBLOCK) || (errno != EAGAIN)) {
        LOG_ERROR("unexpected write, [%d:%s]", errno, strerror_thread_local(errno));
      }
    }
  }
  assert(num_wrote >= 0);
  if (static_cast<size_t>(num_wrote) < count) {
    // pending write.
    output_buffer_.append(data + num_wrote, count - num_wrote);
    if (!channel_.is_writing()) {
      channel_.enable_writing();
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
  if (channel_.is_writing()) {
    auto num_wrote = ::write(channel_.get_fd(), output_buffer_.peek(), output_buffer_.readable_bytes());
    if (num_wrote > 0) {
      output_buffer_.retrieve(num_wrote);
      if (output_buffer_.readable_bytes() == 0) {
        channel_.disable_writing();
      }
      // after sending all write, it is ok to do the previous ask of shutdown.
      if (state_ == kDisconnecting) {
        shutdown();
      }
    } else {
      int err = errno;
      LOG_ERROR("::write(sockfd[%d])", channel_.get_fd(), err, strerror_thread_local(err));
    }
  } else {
    LOG_TRACE("sockfd[%d]:No more writing", channel_.get_fd());
  }
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

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
    set_state(kDisconnecting);
    loop_->run_in_loop([this] {
      if (channel_.is_writing()) {
        // we have pending write task.
        return ;
      }
      if (::shutdown(channel_.get_fd(), SHUT_WR) == -1) {
        int err = errno;
        LOG_ERROR("sockfd[%d] failed to shutdown write, [%d:%s]", 
            err, strerror_thread_local(err));
      }
    });
  }
}

}
