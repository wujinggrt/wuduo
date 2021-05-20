#include <iostream>
#include <thread>
#include <cstring>

#include <gtest/gtest.h>

#include "../wuduo/event_loop.h"
#include "../wuduo/epoller.h"
#include "../wuduo/channel.h"

namespace wuduo {

TEST(EventLoopTest, SimpleTimerfdTest) {
  wuduo::EventLoop el;

  auto fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
  wuduo::Channel ch{&el, fd};
  ch.set_read_callback([fd, pch = &ch] {
      (void)pch;
      uint64_t out;
      ::read(fd, &out, sizeof(out));
      std::cout << "in interest list: " << 
        (pch->is_in_interst_list() ? "true " : "false ") << 
        "timerfd\n";
      static int num_called = 3;
      --num_called;
      if (num_called == 0) {
        pch->disable_reading();
        std::cout << "quiting loop\n";
        pch->get_owner_loop()->quit();
        ::close(fd);
      }
  });
  ch.enable_reading();

  itimerspec howlong;
  std::memset(&howlong, 0, sizeof(howlong));
  howlong.it_value.tv_sec = 2;
  howlong.it_interval.tv_sec = 2;
  ::timerfd_settime(fd, 0, &howlong, nullptr);

  el.loop();
}

TEST(EventLoopTest, ConcurrencyRunInLoopTest) {
  wuduo::EventLoop el;
  std::vector<std::thread> threads;
  for (int i = 0; i <10; ++i) {
    threads.push_back(std::thread{[i, p_el = &el] {
        p_el->run_in_loop([i] {
            std::cout << "In thread " << i << '\n';
          });
    }});
  }
  el.run_in_loop([] {
      std::cout << "Run tasks in queue\n";
  });

  // quit timer
  auto fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
  wuduo::Channel ch{&el, fd};
  ch.set_read_callback([fd, pch = &ch] {
      (void)pch;
      uint64_t out;
      ::read(fd, &out, sizeof(out));
      std::cout << "Quit loop\n";
      ::close(fd);
      pch->get_owner_loop()->quit();
  });
  ch.enable_reading();

  itimerspec howlong;
  std::memset(&howlong, 0, sizeof(howlong));
  howlong.it_value.tv_sec = 3;
  ::timerfd_settime(fd, 0, &howlong, nullptr);
  el.loop();
  for (auto& t : threads){
    t.join();
  }
}

}
