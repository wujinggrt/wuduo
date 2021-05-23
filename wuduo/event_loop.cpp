#include <iostream>
#include <cassert>
#include <utility>
#include <stdexcept>

#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "event_loop.h"
#include "channel.h"
#include "log.h"

namespace wuduo {

static const int epfd = ::epoll_create(5);

thread_local EventLoop* loop_in_this_thread = nullptr;

EventLoop::EventLoop()
  : thread_id_{std::this_thread::get_id()},
  looping_{false},
  quit_{false},
  epoller_{this},
  wakeupfd_{::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)},
  wakeup_channel_{this, wakeupfd_},
  timer_queue_{this},
  doing_pending_tasks_{false} {
  assert(loop_in_this_thread == nullptr);
  loop_in_this_thread = this;

  if (wakeupfd_ == -1) {
    throw std::runtime_error("::eventfd() returns -1");
  }
  wakeup_channel_.set_read_callback([this] {
      uint64_t wakeup_read;
      auto num_read = ::read(wakeupfd_, &wakeup_read, sizeof(wakeup_read));
      if (num_read != sizeof(wakeup_read)) {
        std::cerr << "failed to read all in wakeupfd_\n";
      }
  });
  wakeup_channel_.enable_reading();
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
    for (auto channel : active_channels) {
      channel->handle_events();
    }
    do_pending_tasks();
  }
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!in_loop_thread()) {
    wakeup();
  }
}

void EventLoop::run_in_loop(std::function<void()> task) {
  if (in_loop_thread()) {
    task();
    return;
  }
  queue_in_loop(std::move(task));
}

void EventLoop::queue_in_loop(std::function<void()> task) {
  // critical section
  {
    std::scoped_lock guard{mutex_};
    pending_tasks_.push_back(std::move(task));
  }

  // call loop to complete task.
  // other thread may need this to do some tasks,
  // while the loop member function may be blocked on epoll_wait,
  // it is useful to wakeup to do something.
  //
  // if this io thread is doing pending tasks,
  // the tasks previously commited has been retrieved.
  // It's ok to ask to wakup loop to do.
  if (!in_loop_thread() || doing_pending_tasks_) {
    wakeup();
  }
}

void EventLoop::wakeup() {
  uint64_t wakeup_val = 1;
  ssize_t num_write = ::write(wakeupfd_, &wakeup_val, sizeof(wakeup_val));
  if (num_write != sizeof(wakeup_val)) {
    LOG_ERROR("Failed to wakeup()");
  }
}

 std::shared_ptr<Timer> EventLoop::run_at(std::function<void()> callback, Timestamp when) {
   return timer_queue_.add_timer(std::move(callback), when, Microseconds::zero());
}

std::shared_ptr<Timer> EventLoop::run_after(std::function<void()> callback, Microseconds delay) {
  return run_at(std::move(callback), Clock::now() + Microseconds{delay});
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

void EventLoop::do_pending_tasks() {
  std::vector<std::function<void()>> tasks_dump;
  {
    std::scoped_lock guard{mutex_};
    std::swap(tasks_dump, pending_tasks_);
  }
  doing_pending_tasks_ = true;
  for (auto& t : tasks_dump) {
    t();
  }
  doing_pending_tasks_ = false;
}

}
