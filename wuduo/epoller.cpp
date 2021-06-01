#include <stdexcept>
#include <cstring>
#include <iostream>
#include <string>
#include <cassert>

#include <unistd.h>
#include <error.h>
#include <sys/socket.h>


#include "event_loop.h"
#include "channel.h"
#include "epoller.h"
#include "log.h"
#include "util.h"

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
      LOG_FATAL("[%d:%s]", errno, strerror_thread_local(errno));
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

// Channel should be detrermined whether it has already monitored in epollfd_.
void Epoller::update_channel(Channel* channel) {
  owner_loop_->assert_in_loop_thread();
  ::epoll_event ee;
  ee.events = channel->get_events();
  ee.data.ptr = channel;
  int op = get_ctl_op_from(channel);
  std::string operation{op == EPOLL_CTL_ADD ? "ADD" : op == EPOLL_CTL_MOD ? "MOD" : "DEL"};
  LOG_INFO("Epoller::epoll_ctl(), channel->fd[%d], [%s]", channel->fd(), operation.c_str());
  if (::epoll_ctl(epollfd_, op, channel->fd(), &ee) == -1) {
    int err = get_socket_error(channel->fd());
    LOG_ERROR("::epoll_ctl, %s, channel->fd[%d] [%d:%s]", 
        operation.c_str(), channel->fd(), err, strerror_thread_local(err));
  } else {
    set_state_in_interest_list(channel, op);
  }
}

int Epoller::get_ctl_op_from(Channel* channel) const {
  if (!channel->is_in_interst_list()) {
    // channel has never added to epoll.
    assert(!channel->has_none_events());
    return EPOLL_CTL_ADD;
  }
  return channel->has_none_events() ?
    EPOLL_CTL_DEL : EPOLL_CTL_MOD;
}

void Epoller::set_state_in_interest_list(Channel* channel, int op) const {
  if (!channel->is_in_interst_list()) {
    assert(op == EPOLL_CTL_ADD);
    channel->set_in_interest_list(true);
    return ;
  }
  if (channel->has_none_events()) {
    assert(op == EPOLL_CTL_DEL);
    channel->set_in_interest_list(false);
    return;
  }
  assert(op == EPOLL_CTL_MOD);
  channel->set_in_interest_list(true);
}

}
