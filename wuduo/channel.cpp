#include <thread>
#include <iostream>

#include <sys/epoll.h>

#include "channel.h"
#include "event_loop.h"
#include "log.h"

namespace wuduo {

Channel::Channel(EventLoop* loop, int fd)
  : loop_{loop}, 
  fd_{fd}, 
  events_{0},
  revents_{0},
  in_interest_list_{false} {}

Channel::~Channel() {}

void Channel::handle_events() {
  loop_->assert_in_loop_thread();
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    LOG_INFO("Channel::handle_events() EPOLLHUP");
    if (close_callback_) {
      close_callback_();
    }
  }
  if (revents_ & EPOLLERR) {
    if (error_callback_) {
      error_callback_();
    }
  }
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (read_callback_) {
      read_callback_();
    }
  }
  if (revents_ & EPOLLOUT) {
    if (write_callback_) {
      write_callback_();
    }
  }
}

void Channel::enable_reading() {
  events_ |= EPOLLIN | EPOLLPRI;
  update();
}

void Channel::enable_writing() {
  events_ |= EPOLLOUT;
  update();
}

void Channel::disable_reading() {
  events_ &= ~(EPOLLIN | EPOLLPRI);
  update();
}

void Channel::disable_writing() {
  events_ &= ~EPOLLOUT;
  update();
}

void Channel::disable_all() {
  events_ = 0;
  update();
}

bool Channel::is_reading() {
  return events_ & (EPOLLIN | EPOLLPRI);
}

bool Channel::is_writing() {
  return events_ & EPOLLOUT;
}

void Channel::update() {
  loop_->update_channel(this);
  in_interest_list_ = !is_none_events();
}

}
