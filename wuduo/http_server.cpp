#include <cassert>
#include <cstring>
#include <utility>
#include <tuple>

#include <unistd.h>

#include "http_server.h"
#include "tcp_connection.h"
#include "log.h"
#include "util.h"

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

  return RequestLine::from(method, url, line);
}

std::optional<RequestLine> RequestLine::from(std::string_view method, 
                                             std::string_view url, 
                                             std::string_view version) {
  RequestLine ret;

  if (method == "GET") {
    ret.method = RequestLine::Method::kGet;
  } else if (method == "HEAD") {
    ret.method = RequestLine::Method::kHead;
  } else if (method == "POST") {
    ret.method = RequestLine::Method::kPost;
  } else {
    return std::nullopt;
  }

  ret.url = std::string{url};

  if (version == "HTTP/1.0") {
    ret.version = RequestLine::Version::kHttp10;
  } else if (version == "HTTP/1.1") {
    ret.version = RequestLine::Version::kHttp11;
  } else {
    return std::nullopt;
  }

  return ret;
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
  auto& metadata = [this, &conn] () -> HttpConnectionMetadata& {
    std::scoped_lock guard{mutex_};
    return connection_metadata_[conn];
  } ();
  if (msg.empty()) {
    connection_metadata_.erase(conn);
    return ;
  }
  if (conn->is_disconnected()) {
    metadata.in_buffer.clear();
    return ;
  }
  auto num_read = msg.size();
  if (metadata.parsing_state == 
      HttpConnectionMetadata::ParsingState::kRequestLine) {
    auto pos_cr_lf = msg.find('\r');
    if (pos_cr_lf == std::string::npos) {
      metadata.in_buffer += std::move(msg);
      LOG_INFO("sockfd[%d] num_read[%u], wait for further read",
          conn->get_channel()->get_fd(), num_read);
      return ;
    }
    pos_cr_lf = metadata.in_buffer.size() + pos_cr_lf;
    metadata.in_buffer += std::move(msg);
    auto request_line = RequestLine::from(
        std::string_view{metadata.in_buffer.data(), pos_cr_lf + 1});
    if (!request_line) {
      LOG_ERROR("sockfd[%d] failed to parse request_line", conn->get_channel()->get_fd());
      // TODO: send error page and handle close.
      return ;
    }
    metadata.parsing_state = HttpConnectionMetadata::ParsingState::kFinished;
    metadata.request_line = request_line.value();
  }

  if (metadata.parsing_state == HttpConnectionMetadata::ParsingState::kFinished) {
    int connection_fd = conn->get_channel()->get_fd();
    LOG_INFO("Request message(from[%d]):\n%sRespond:\n%s",
        connection_fd, metadata.in_buffer.c_str(), kHelloWorld.data());
    if (::write(connection_fd, kHelloWorld.data(), kHelloWorld.size()) == -1) {
      LOG_ERROR("connection_fd[%d] failed to write [%d:%s]",
          connection_fd, errno, strerror_thread_local(errno));
    }
    LOG_INFO("sockfd[%d] - Message responded", connection_fd);
  }
}

}
