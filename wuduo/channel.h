#pragma once

#include <functional>
#include <cstdint>

#include "noncopyable.h"

namespace wuduo {

class EventLoop;

class Channel : noncopyable {
 public:
  using ReadCallback = std::function<void()>;
  using WriteCallback = std::function<void()>;
  using CloseCallback = std::function<void()>;
  using ErrorCallback = std::function<void()>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void set_read_callback(const ReadCallback& cb) {
    read_callback_ = cb;
  }

  void set_write_callback(const WriteCallback& cb) {
    write_callback_ = cb;
  }

  void set_close_callback(const CloseCallback& cb) {
    close_callback_ = cb;
  }

  void set_error_callback(const ErrorCallback& cb) {
    error_callback_ = cb;
  }

  void handle_events();

  EventLoop* owner_loop() const { return loop_; }
  int fd() const { return fd_; }
  uint32_t events() const { return events_; }
  void set_revents(uint32_t e) { revents_ = e; }
  bool has_none_events() const { return events_ == 0; }
  bool is_in_interst_list() const { return in_interest_list_; }
  void set_in_interest_list(bool in) { in_interest_list_ = in; }

  void enable_reading();
  void enable_writing();
  void disable_reading();
  void disable_writing();
  // can not be called if this channel has not called either of enable function,
  // only called after the channel was added to epoll's interest list.
  // The epoller will remove fd from interest list.
  void disable_all();

  bool is_reading();
  bool is_writing();

 private:
  void update();

  EventLoop* loop_;
  int fd_;
  uint32_t events_;
  uint32_t revents_;
  // used by Epoller
  // state machine:
  bool in_interest_list_;

  ReadCallback read_callback_;
  WriteCallback write_callback_;
  CloseCallback close_callback_;
  ErrorCallback error_callback_;
};

}
