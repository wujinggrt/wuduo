#pragma once

#include <utility>
#include <set>
#include <memory>

#include <sys/timerfd.h>

#include "noncopyable.h"
#include "timer.h"
#include "channel.h"

namespace wuduo {

class EventLoop;

class TimerQueue : noncopyable {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  // returns a timer to user to cancel
  std::shared_ptr<Timer> add_timer(TimerCallback cb, Timestamp when, Microseconds interval);

 private:
  using Entry = std::pair<Timestamp, std::shared_ptr<Timer>>;
  static inline const std::function<bool(const Entry&, const Entry&)> 
    entry_comparator = 
      [] (const Entry& lhs, const Entry& rhs) { 
            return std::make_pair(lhs.first, lhs.second.get()) < 
              std::make_pair(rhs.first, rhs.second.get());
      };
  using EntrySet = std::set<Entry, decltype(entry_comparator)>;

  void handle_read();
  std::vector<Entry> get_expired(Timestamp now);
  void reset_alarm(Timestamp near_future);

  EventLoop* loop_;
  int timerfd_;
  Channel channel_;
  EntrySet timers_;
};

}
