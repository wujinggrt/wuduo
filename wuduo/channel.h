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

  EventLoop* get_owner_loop() const { return loop_; }
  int get_fd() const { return fd_; }
  uint32_t get_events() const { return events_; }
  void set_revents(uint32_t e) { revents_ = e; }
  bool is_none_events() const { return events_ == 0; }
  bool is_in_interst_list() const { return in_interest_list_; }

  void enable_reading();
  void enable_writing();
  void disable_reading();
  void disable_writing();
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
  bool in_interest_list_;

  ReadCallback read_callback_;
  WriteCallback write_callback_;
  CloseCallback close_callback_;
  ErrorCallback error_callback_;
};

}
