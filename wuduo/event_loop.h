#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <functional>

#include "noncopyable.h"
#include "epoller.h"
#include "channel.h"
#include "timer.h"
#include "timer_queue.h"

namespace wuduo {

class Channel;

class EventLoop : noncopyable {
 public:
   EventLoop();

   ~EventLoop();
   
   void loop();
   void quit();

   void run_in_loop(std::function<void()> task);
   void queue_in_loop(std::function<void()> task);

   void wakeup();

   std::shared_ptr<Timer> run_at(std::function<void()> callback, Timestamp when);
   std::shared_ptr<Timer> run_after(std::function<void()> callback, Microseconds delay);

   void update_channel(Channel* channel);

   bool in_loop_thread() const;

   void assert_in_loop_thread() const;
   void assert_not_in_loop_thread() const;

   static EventLoop* get_event_loop_in_this_thread();

 private:
   void do_pending_tasks();
   
   std::thread::id thread_id_;
   bool looping_;
   bool quit_;
   Epoller epoller_;

   // used for inter-thread communication, in a way that epoll_wait could monitor.
   int wakeupfd_;
   Channel wakeup_channel_;
   TimerQueue timer_queue_;
   bool doing_pending_tasks_;
   std::mutex mutex_;
   std::vector<std::function<void()>> pending_tasks_;
};

}
