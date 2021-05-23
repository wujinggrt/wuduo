#include <cassert>

#include "event_loop.h"
#include "event_loop_thread.h"

namespace wuduo {

EventLoopThread::EventLoopThread()
  : loop_{nullptr},
  started_{false},
  exiting_{false}
{}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    thread_->join();
  }
}

EventLoop* EventLoopThread::start() {
  assert(!started_);
  started_ = true;
  thread_ = std::make_unique<std::thread>([this] {
      EventLoop loop;
      {
        std::scoped_lock guard{mutex_};
        loop_ = &loop;
        cond_.notify_one();
      }
      loop.loop();
      loop_ = nullptr;
    }
);
  // wait until loop_ was initialized.
  {
    std::unique_lock guard{mutex_};
    cond_.wait(guard, [this] { return loop_ != nullptr; });
  }
  return loop_;
}

}
