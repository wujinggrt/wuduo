#pragma once

#include <vector>

#include <sys/epoll.h>

#include "noncopyable.h"

namespace wuduo {

class EventLoop;
class Channel;

class Epoller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  Epoller(EventLoop* loop);
  ~Epoller();

  ChannelList wait();

  void update_channel(Channel* channel);
  void remove_channel(Channel* channel);

 private:

  EventLoop* owner_loop_;
  std::vector<epoll_event> events_;
  int epollfd_;

};

}
