#include <cassert>
#include <future>

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
  // no 100% race-free
  if ((loop_ != nullptr) && (thread_ != nullptr)) {
    loop_->quit();
    thread_->join();
  }
}

EventLoop* EventLoopThread::start() {
  assert(!started_);
  started_ = true;
  std::promise<EventLoop*> loop_promise;
  thread_ = std::make_unique<std::thread>([this, ptr_loop_promise = &loop_promise] () mutable {
      EventLoop loop;
      ptr_loop_promise->set_value(&loop);
      // never used later.
      ptr_loop_promise = nullptr;
      loop.loop();
      // exited, no longer needed
      loop_ = nullptr;
    }
);
  return loop_promise.get_future().get();
}

}
