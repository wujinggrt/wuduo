#include <cassert>
#include <cstring>
#include <utility>
#include <tuple>

#include <unistd.h>

#include "http_server.h"
#include "tcp_connection.h"
#include "log.h"
#include "util.h"

namespace {

using namespace wuduo;

[[maybe_unused]] void on_http_message_sample(const TcpConnectionPtr& conn, std::string msg) {
  int connection_fd = conn->get_channel()->get_fd();
  LOG_INFO("Request message(from[%d]):\n%s", connection_fd, msg.c_str());
  LOG_INFO("Respond message:\n%s", kHelloWorld.data());
  auto request_line = RequestLine::from(msg);
  LOG_INFO("Parsed request line[method:%s url:%s version:%s]", request_line->method.c_str(), request_line->url.c_str(), request_line->version.c_str());
  if (::write(connection_fd, kHelloWorld.data(), kHelloWorld.size()) == -1) {
    LOG_ERROR("connection_fd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
  }
  LOG_INFO("sockfd[%d] - Message responded", connection_fd);
}

}

namespace wuduo {

std::optional<RequestLine> RequestLine::from(std::string_view line) {
  auto pos_cr_lf = line.find('\r');
  if (pos_cr_lf != std::string_view::npos) {
    line.remove_suffix(line.size() - pos_cr_lf);
  }
  auto space_pos = line.find(' ');
  if (space_pos == std::string_view::npos) {
    return std::nullopt;
  }
  std::string_view method = line.substr(0, space_pos);
  assert(space_pos + 1 < line.size());
  line.remove_prefix(space_pos + 1);
  space_pos = line.find(' ');
  if (space_pos == std::string_view::npos) {
    return std::nullopt;
  }
  std::string_view url = line.substr(0, space_pos);
  line.remove_prefix(space_pos + 1);
  assert(space_pos + 1 < line.size());
  return std::make_optional<RequestLine>({std::string{method}, std::string{url}, std::string{line}});
}

HttpServer::HttpServer(EventLoop* loop, InetAddress address)
  : loop_{loop},
  local_{address},
  server_{loop, local_}
{}

void HttpServer::start() {
  server_.set_message_callback([this] (auto& conn, auto msg) {
    handle_read(conn, std::move(msg));
  });
  server_.start();
}

void HttpServer::handle_read(const TcpConnectionPtr& conn, std::string msg) {
  // EOF, peer closed.
  auto&& [in_buffer, out_buffer, parsing_state] = [this, &conn] {
    std::scoped_lock guard{mutex_};
    auto& metadata = connection_metadata_[conn];
    return std::tie(metadata.in_buffer, metadata.out_buffer, metadata.parsing_state);
  } ();
  (void) out_buffer;
  if (msg.empty()) {
    connection_metadata_.erase(conn);
    return ;
  }
  if (conn->is_disconnected()) {
    in_buffer.clear();
    return ;
  }
  [[maybe_unused]] auto num_read = msg.size();
  LOG_INFO("sockfd[%d] num_read[%u]", conn->get_channel()->get_fd(), num_read);
  if (parsing_state == TcpConnectionMetadata::ParsingState::kRequestLine) {
    auto pos_cr_lf = msg.find('\r');
    if (pos_cr_lf == std::string::npos) {
      in_buffer += std::move(msg);
      LOG_INFO("sockfd[%d] num_read[%u], wait for further read", conn->get_channel()->get_fd(), num_read);
      return ;
    }
    pos_cr_lf = in_buffer.size() + pos_cr_lf - 1;
    in_buffer += std::move(msg);
    auto request_line = RequestLine::from(in_buffer);
    LOG_INFO("Parsed request line[method:%s url:%s version:%s]", 
        request_line->method.c_str(), request_line->url.c_str(), request_line->version.c_str());
    parsing_state = TcpConnectionMetadata::ParsingState::kFinished;
  }

  if (parsing_state == TcpConnectionMetadata::ParsingState::kFinished) {
    int connection_fd = conn->get_channel()->get_fd();
    LOG_INFO("Request message(from[%d]):\n%s", connection_fd, in_buffer.c_str());
    LOG_INFO("Respond message:\n%s", kHelloWorld.data());
    if (::write(connection_fd, kHelloWorld.data(), kHelloWorld.size()) == -1) {
      LOG_ERROR("connection_fd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
    }
    LOG_INFO("sockfd[%d] - Message responded", connection_fd);
  }
}

}
