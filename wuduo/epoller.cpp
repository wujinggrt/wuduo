#include <stdexcept>
#include <iostream>

#include <unistd.h>
#include <error.h>

#include "event_loop.h"
#include "channel.h"
#include "epoller.h"

namespace wuduo {

static constexpr const int kDefaultEventsOutputSize = 128;

Epoller::Epoller(EventLoop* loop)
  : owner_loop_{loop},
  events_{kDefaultEventsOutputSize},
  epollfd_{::epoll_create1(EPOLL_CLOEXEC)} {}

Epoller::~Epoller() {
  ::close(epollfd_);
}

Epoller::ChannelList Epoller::wait() {
  owner_loop_->assert_in_loop_thread();
  ChannelList active_channels;
  const int max_events = static_cast<int>(events_.size());
  const int num_ready = ::epoll_wait(epollfd_, events_.data(), max_events, -1);
  if (num_ready == -1) {
    if (errno != EINTR) {
      throw std::runtime_error("Epoller::epoll_wait()");
    }
  }
  if (num_ready > 0) {
    for (int i = 0; i < num_ready; ++i) {
      auto channel = static_cast<Channel*>(events_[i].data.ptr);
      channel->set_revents(events_[i].events);
      active_channels.push_back(channel);
    }
    if (num_ready == max_events) {
      events_.resize(1.5 * max_events);
    }
  }
  return active_channels;
}

// Channel should be detrermined whether it has already monitored in epollfd_
void Epoller::update_channel(Channel* channel) {
  owner_loop_->assert_in_loop_thread();
  int op = 0;
  ::epoll_event ee;
  ee.events = channel->get_events();
  ee.data.ptr = channel;
  if (!channel->is_in_interst_list()) {
    op = EPOLL_CTL_ADD;
  } else {
    op = channel->is_none_events() ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
  }
  std::cerr << "Epoller::epoll_ctl() [" << 
    (op == EPOLL_CTL_ADD ? "Add" : op == EPOLL_CTL_MOD ? "Mod" : "Del") << "]\n";;
  if (::epoll_ctl(epollfd_, op, channel->get_fd(), &ee) != 0) {
    if (errno != EINTR) {
      std::string msg = [] () -> std::string {
        if (errno == EEXIST) {
          return "Existed fd";
        } else if (errno == ENOENT) {
          return "ENOENT";
        } else {
          return "Others";
        }
      } ();
      throw std::logic_error("Epoller::epoll_ctl() failed " + msg);
    }
  }
}

}
