#include <thread>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include <gtest/gtest.h>

#include "../wuduo/channel.h"
#include "../wuduo/event_loop.h"

TEST(TimerQueueTest, SingleThreadTest) {
  wuduo::EventLoop* g_el;
  wuduo::EventLoop el;
  g_el = &el;

  g_el->run_after([g_el] {
      std::cout << "Run after " << 1 << '\n';
      std::cout << "Asking el to quit\n";
      g_el->quit();
      }, wuduo::Seconds{1});
  el.loop();
}

TEST(TimerQueueTest, ConcurrencyTest) {

  wuduo::EventLoop* g_el;
  wuduo::EventLoop el;
  g_el = &el;
#if 0
  std::thread t1{[&g_el] {
    wuduo::EventLoop el;
    g_el = &el;
 4j   std::cout << "thread1 started\n";
    el.loop();
  }};
#endif
#if 0
  for (int i = 0; i < 1; i++) {
    std::thread t1{[&] {
      g_el->run_in_loop([] {
          std::cout << "Run in other thr\n";
          });
      g_el->run_after([i] {
          std::cout << "Run after " << i << '\n';
          }, wuduo::Seconds{i});
    }};
    t1.detach();
  }
#endif
}
