#pragma once

#include <memory>
#include <utility>

#include "event_loop.h"
#include "channel.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "callbacks.h"

namespace wuduo {

class TcpConnection : noncopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
 public:
   TcpConnection(EventLoop* loop, int sockfd, InetAddress peer);
   ~TcpConnection();

   void set_message_callback(MessageCallback cb) {
     message_callback_ = std::move(cb);
   }
   void set_close_connection_callback(CloseCallback cb) {
     close_callback_ = std::move(cb);
   }

   // called only once and in this loop_, so it may be defered to established.
   // state: connecting -> connected
   void established();

 private:
   void handle_read();
   void handle_close();
   void handle_error();

   EventLoop* loop_;
   Channel channel_;
   const InetAddress peer_;
   bool reading_;
   MessageCallback message_callback_;
   CloseCallback close_callback_;
};

}
