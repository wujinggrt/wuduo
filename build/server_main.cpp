#include <iostream>
#include <string>
#include <unistd.h>

#include <sys/socket.h>

#include "../wuduo/tcp_server.h"
#include "../wuduo/event_loop.h"
#include "../wuduo/inet_address.h"

int main() {
  wuduo::EventLoop loop;
  wuduo::InetAddress address{12000};
  wuduo::TcpServer server{&loop, address};
  server.start();
  loop.loop();
}
