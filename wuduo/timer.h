#pragma once

#include <functional>
#include <chrono>

#include "noncopyable.h"

namespace wuduo {

using TimerCallback = std::function<void()>;

using Clock = std::chrono::system_clock;
using Seconds = std::chrono::seconds;
using Microseconds = std::chrono::microseconds;
using Nanoseconds = std::chrono::nanoseconds;
using Timestamp = std::chrono::time_point<Clock>;

inline constexpr auto AsSeconds = 
  [] (auto interval) {
    return std::chrono::duration_cast<Seconds>(interval);
};

inline constexpr auto AsMicroseconds = 
  [] (auto interval) {
    return std::chrono::duration_cast<Microseconds>(interval);
};

inline constexpr auto AsNanoseconds = 
  [] (auto interval) {
    return std::chrono::duration_cast<Nanoseconds>(interval);
};

inline constexpr auto timestamp_now = 
  [] { return std::chrono::time_point_cast<Microseconds>(Clock::now()); };

class Timer : noncopyable {
 public:
  Timer(TimerCallback cb, Timestamp when, Microseconds interval = Microseconds::zero())
    : callback_{std::move(cb)},
    when_{std::move(when)},
    interval_{std::move(interval)},
    repeat_{interval != Microseconds::zero()} {}

  ~Timer() = default;

  void run() {
    if (callback_) {
      callback_();
    }
  }
  Timestamp get_when() const { return when_; }
  Microseconds get_interval() { return interval_; }
  bool is_expired(Timestamp now) const { return now >= when_; }
  
 private:
  TimerCallback callback_;
  Timestamp when_;
  Microseconds interval_;
  bool repeat_;

};

}
