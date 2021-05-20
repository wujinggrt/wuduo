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
      std::printf("Run after 5000us\n");
      std::cout << "Asking el to quit\n";
      g_el->quit();
      }, wuduo::Microseconds{5000});

  g_el->run_at([] {
      std::printf("Run after 1000us\n");
      std::printf("Hello loop\n");
  }, wuduo::Clock::now() + wuduo::Microseconds{1000});
  el.loop();
}

TEST(TimerQueueTest, ConcurrencyTest) {
  wuduo::EventLoop* g_el;
  wuduo::EventLoop el;
  g_el = &el;
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; i++) {
    std::thread t1{[i, g_el] {
      g_el->run_in_loop([i] {
          std::printf("Threads %d running\n", i);
        });
      // since threads runs concurrency, we can not determine that the i-th thread runs in the i-th order.
      // maybe in a order [0, 2, 3, 4, 1];
      g_el->run_after([i] {
          std::printf("Run after 50 + %d * 50\n", i);
          }, wuduo::Microseconds{50 + i * 50});
    }};
    threads.push_back(std::move(t1));
  }

  // quit after 5s
  el.run_after([g_el] {
      std::printf("Quiting loop\n");
      g_el->quit();
  }, wuduo::Seconds{4});

  el.loop();

  for (auto& t : threads) {
    t.join();
  }
}
