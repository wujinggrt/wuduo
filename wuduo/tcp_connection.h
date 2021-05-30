#pragma once

#include <memory>
#include <utility>
#include <string_view>
#include <string>
#include <any>

#include "wuduo/event_loop.h"
#include "wuduo/channel.h"
#include "wuduo/inet_address.h"
#include "wuduo/buffer.h"
#include "wuduo/noncopyable.h"
#include "wuduo/callbacks.h"

namespace wuduo {

class TcpConnection : noncopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
    friend class TcpServer;
 public:
   TcpConnection(EventLoop* loop, int sockfd, InetAddress peer);
   ~TcpConnection();

   void set_connection_callback(ConnectionCallback cb) {
     connection_callback_ = std::move(cb);
   }
   void set_message_callback(MessageCallback cb) {
     message_callback_ = std::move(cb);
   }
   void set_close_callback(CloseCallback cb) {
     close_callback_ = std::move(cb);
   }
   bool is_connected() const { return state_ == kConnected; }
   bool is_disconnected() const { return state_ == kDisconnected; }

   const Channel* channel() const { return &channel_; }
   EventLoop* loop() const { return loop_; }
   void set_context(std::any context) { context_ = std::move(context); }
   std::any* context() { return &context_; }

   // called only once and in this loop_, so it may be defered to established.
   // state: connecting -> connected, via server.
   void established();

   void send_in_loop(std::string_view data);

   void handle_read();
   void handle_write();
   // this can be called only once.
   void handle_close();
   void handle_error();

   void force_close();

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
 private:
   // the last call to channel, then it will be destruct.
   void destroyed();

   enum State { kConnecting, kConnected, kDisconnecting, kDisconnected };

   void set_state(State s) { state_ = s; }

   EventLoop* loop_;
   State state_;
   Channel channel_;
   const InetAddress peer_;
   bool reading_;
   ConnectionCallback connection_callback_;
   MessageCallback message_callback_;
   CloseCallback close_callback_;
   Buffer in_buffer_;
   Buffer out_buffer_;
   std::any context_;
};

}
