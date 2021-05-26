#pragma once

#include <memory>
#include <utility>
#include <string_view>
#include <string>

#include "event_loop.h"
#include "channel.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "callbacks.h"

namespace wuduo {

class TcpConnection : noncopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
    friend class HttpServer;
 public:
   TcpConnection(EventLoop* loop, int sockfd, InetAddress peer);
   ~TcpConnection();

   void set_message_callback(MessageCallback cb) {
     message_callback_ = std::move(cb);
   }
   void set_close_callback(CloseCallback cb) {
     close_callback_ = std::move(cb);
   }
   bool is_connected() const { return state_ == kConnected; }
   bool is_disconnected() const { return state_ == kDisconnected; }

   Channel* get_channel() { return &channel_; }
   EventLoop* get_loop() const { return loop_; }

   // called only once and in this loop_, so it may be defered to established.
   // state: connecting -> connected, via server.
   void established();
   // the last call to channel, then it will be destruct.
   void destroyed();

   void send_in_loop(std::string_view data);

   void handle_read();
   void handle_write();
   // this can be called only once.
   void handle_close();
   void handle_error();

 private:
   enum State { kConnecting, kConnected, kDisconnecting, kDisconnected };

   void set_state(State s) { state_ = s; }
   std::string get_state_string() {
     switch (state_) {
       case kConnecting: return "kConnecting";
       case kConnected: return "kConnected";
       case kDisconnecting: return "kDisconnecting";
       case kDisconnected: return "kDisconnected";
       default: break;
     }
     return "unexpected state";
   }

   EventLoop* loop_;
   State state_;
   Channel channel_;
   const InetAddress peer_;
   bool reading_;
   MessageCallback message_callback_;
   CloseCallback close_callback_;
};

}
