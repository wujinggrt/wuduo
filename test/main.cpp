#include <chrono>
#include <iostream>
#include <thread>
#include <cstring>

#include <unistd.h>
#include <sys/timerfd.h>

#include "event_loop.h"
#include "channel.h"
#include "epoller.h"

using namespace wuduo;

#if 1
int main() {
  wuduo::EventLoop loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
  Channel channel(&loop, timerfd);
  channel.set_read_callback([loop_ptr = &loop]{
      std::cout << "Time out!\n";
      loop_ptr->quit();
  });
  channel.enable_reading();

  itimerspec howlong;
  std::memset(&howlong, 0, sizeof(howlong));
  howlong.it_value.tv_sec = 2;
  ::timerfd_settime(timerfd, 0, &howlong, nullptr);

  loop.loop();
  ::close(timerfd);
  return 0;
}
#endif
