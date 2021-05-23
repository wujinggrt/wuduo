#include <cassert>
#include <future>

#include "event_loop.h"
#include "event_loop_thread.h"

namespace wuduo {

EventLoopThread::EventLoopThread()
  : loop_{nullptr}
#if 0
  started_{false},
  exiting_{false}
#endif
{}

EventLoopThread::~EventLoopThread() {
#if 0
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    thread_->join();
  }
#endif
}

EventLoop* EventLoopThread::start() {
#if 0
  assert(!started_);
  started_ = true;
#endif
#if 0
  std::promise<EventLoop*> loop_promise;
  thread_ = std::make_unique<std::thread>([this, &loop_promise] {
#endif
#if 0
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
#endif
#if 0
  auto loop_future = loop_promise.get_future();
  loop_ = loop_future.get();
#endif
#if 0
  // wait until loop_ was initialized.
  {
    std::unique_lock guard{mutex_};
    cond_.wait(guard, [this] { return loop_ != nullptr; });
  }
#endif
  return loop_;
}

}
