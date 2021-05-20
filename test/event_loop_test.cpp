#include <iostream>
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

TEST(EventLoopTest, DISABLED_ChannelTest) {
  wuduo::EventLoop el;
  el.run_in_loop([] {
      std::cout << "Run tasks in queue\n";
  });
}

}
