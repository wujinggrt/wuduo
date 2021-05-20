#include <stdexcept>
#include <chrono>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <memory>

#include <sys/timerfd.h>
#include <unistd.h>

#include "timer_queue.h"
#include "event_loop.h"

namespace wuduo {

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_{loop},
  timerfd_{::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK)},
  channel_{loop, timerfd_},
  timers_{entry_comparator} {
  if (timerfd_ == -1) {
    throw std::runtime_error("::timerfd_create returns -1");
  }
  channel_.set_read_callback([this]{ handle_read(); });
  channel_.enable_reading();
}

TimerQueue::~TimerQueue() {
  if (timerfd_ != -1) {
    ::close(timerfd_);
  }
}

std::shared_ptr<Timer> TimerQueue::add_timer(TimerCallback cb, Timestamp when, Microseconds interval) {
  std::cerr << "Adding timer\n";
  auto timer = std::make_shared<Timer>(std::move(cb), when, interval);
  loop_->run_in_loop([&] {
#if 1
      std::cerr << "Adding timer, run_in_loop\n";
      if (timers_.empty() || when < timers_.begin()->first) {
        reset_alarm(when);
      }
      std::cerr << "Adding timer, reseted\n";
      timers_.emplace(when, timer);
      std::cerr << "timers_ emplaced\n";
#endif
  });
  return timer;
}

void TimerQueue::handle_read() {
  loop_->assert_in_loop_thread();
  std::cerr << "Handling read in timer queue\n";
  uint64_t timer_expired;
  ssize_t num_read = ::read(timerfd_, &timer_expired, sizeof(timer_expired));
  if (num_read != sizeof(timer_expired)) {
    std::cerr << "::read(timerfd_, &timer_expired, sizeof(timer_expired)) != sizeof(timer_expired)\n";
  }

  for (const auto& e : get_expired(Clock::now())) {
    e.second->run();
  }
  if (!timers_.empty()) {
    reset_alarm(timers_.begin()->first);
  }
}

std::vector<TimerQueue::Entry> TimerQueue::get_expired(Timestamp now) {
  // the deleter of the latter shared_ptr dose nothing, since no memory was allocated.
  auto first_valid = timers_.lower_bound(
      std::make_pair(now, 
        std::shared_ptr<Timer>{reinterpret_cast<Timer*>(UINTPTR_MAX), [](auto) {}}));
  std::vector<TimerQueue::Entry> expired{timers_.begin(), first_valid};
  timers_.erase(timers_.begin(), first_valid);
  return expired;
}

void TimerQueue::reset_alarm(Timestamp near_future) {
  // reset the timer
  itimerspec new_time;
  std::memset(&new_time, 0, sizeof(new_time));
  auto delta = near_future - Clock::now();
  new_time.it_value.tv_sec = AsSeconds(delta).count();
  new_time.it_value.tv_nsec = 
    AsNanoseconds(delta).count() % AsNanoseconds(Seconds{1}).count();
  ::timerfd_settime(timerfd_, 0, &new_time, nullptr);
}

}
