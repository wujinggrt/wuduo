#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace wuduo {

class EventLoop;

class EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();

  // can not called twice.
  EventLoop* start();

 private:
  EventLoop* loop_;
#if 0
  bool started_;
  bool exiting_;
  std::unique_ptr<std::thread> thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
#endif
};

}
