#pragma once

#include <thread>

#include "noncopyable.h"
#include "epoller.h"

namespace wuduo {

class Channel;

class EventLoop : noncopyable {
 public:
   EventLoop();

   ~EventLoop();
   
   void loop();
   void quit();

   void update_channel(Channel* channel);

   bool in_loop_thread() const;

   void assert_in_loop_thread() const;
   void assert_not_in_loop_thread() const;

   static EventLoop* get_event_loop_in_this_thread();

 private:
   std::thread::id thread_id_;
   bool looping_;
   bool quit_;
   Epoller epoller_;
};

}
