#include <iostream>
#include <cassert>

#include <sys/epoll.h>
#include <sys/poll.h>

#include "event_loop.h"
#include "channel.h"

namespace wuduo {

static const int epfd = ::epoll_create(5);

thread_local EventLoop* loop_in_this_thread = nullptr;

EventLoop::EventLoop()
  : thread_id_{std::this_thread::get_id()},
  looping_{false},
  quit_{false},
  epoller_{this} {
  assert(loop_in_this_thread == nullptr);
  loop_in_this_thread = this;
}

EventLoop::~EventLoop() {
  assert(loop_in_this_thread == this);
  loop_in_this_thread = nullptr;
}

void EventLoop::loop() {
  assert_in_loop_thread();
  looping_ = true;
  for (; !quit_;) {
    const auto active_channels = epoller_.wait();
    std::cout << active_channels.size() << '\n';
    for (auto channel : active_channels) {
      channel->handle_events();
    }
  }
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
}

void EventLoop::update_channel(Channel* channel) {
  assert(channel->get_owner_loop() == this);
  assert_in_loop_thread();
  epoller_.update_channel(channel);
}

 bool EventLoop::in_loop_thread() const {
   return thread_id_ == std::this_thread::get_id();
 }

void EventLoop::assert_in_loop_thread() const {
  assert(in_loop_thread());
}

void EventLoop::assert_not_in_loop_thread() const {
  assert(!in_loop_thread());
}

EventLoop* EventLoop::get_event_loop_in_this_thread() {
  return loop_in_this_thread;
}

}
