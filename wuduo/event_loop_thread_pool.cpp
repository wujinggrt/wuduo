#include <cassert>

#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"
#include "event_loop.h"

namespace wuduo {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, int num_threads)
  : base_loop_{base_loop},
  num_threads_{num_threads},
  started_{false},
  next_{0} {
  assert(num_threads >= 0);
}

void EventLoopThreadPool::start() {
  base_loop_->assert_in_loop_thread();
  started_ = true;
  worker_threads_.reserve(num_threads_);
  workers_.reserve(num_threads_);
  for (int i = 0; i < num_threads_; ++i) {
    worker_threads_.emplace_back(std::make_unique<EventLoopThread>());
    workers_.push_back(worker_threads_[i]->start());
  }
}

EventLoop* EventLoopThreadPool::get_next_loop() {
  base_loop_->assert_in_loop_thread();
  assert(started_);
  if (workers_.empty()) {
    return base_loop_;
  }
  int loop_index = next_;
  next_ = (next_ + 1) % num_threads_;
  return workers_[loop_index];
}

}
