#pragma once

#include <memory>
#include <vector>

#include "noncopyable.h"

namespace wuduo {

class EventLoop;

class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  EventLoopThreadPool(EventLoop* base_loop, int num_threads);

  // can be called only within base_loop_.loop.
  void start();

  EventLoop* next_loop();

 private:
  EventLoop* base_loop_;
  const int num_threads_;
  bool started_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> worker_threads_;
  std::vector<EventLoop*> workers_;
};

}
