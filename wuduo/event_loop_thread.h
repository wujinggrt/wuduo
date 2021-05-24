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
  bool started_;
  bool exiting_;
  std::unique_ptr<std::thread> thread_;
};

}
