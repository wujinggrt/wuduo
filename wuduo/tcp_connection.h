#pragma once

#include <memory>

#include "event_loop.h"
#include "channel.h"
#include "inet_address.h"
#include "noncopyable.h"

namespace wuduo {

class TcpConnection : noncopyable {
 public:
   TcpConnection(EventLoop* loop, int sockfd, InetAddress peer);
   ~TcpConnection();

 private:
   EventLoop* loop_;
   Channel channel_;
   const InetAddress peer_;
   bool reading_;
};

}
