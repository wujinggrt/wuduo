#pragma once

#include <functional>
#include <string>
#include <memory>

namespace wuduo {

class InetAddress;

// the user should mannualy close the socket that connected the peer client.
using NewConnectionCallback = std::function<void(int connect_fd, InetAddress peer)>;
class TcpConnection;
class Buffer;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

}
