#include <cassert>
#include <cstring>
#include <utility>
#include <tuple>

#include <unistd.h>
#include <sys/uio.h>

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

std::string RequestLine::to_string(const RequestLine& line) {
  std::string ret;

  switch (line.method) {
    case RequestLine::Method::kGet: ret += "GET"; break;
    case RequestLine::Method::kHead: ret += "HEAD"; break;
    case RequestLine::Method::kPost: ret += "POST"; break;
    default: ret += "UNKNOWN"; break;
  }

  ret += " " + line.url + " ";

  switch (line.version) {
    case RequestLine::Version::kHttp10: ret += "HTTP/1.0"; break;
    case RequestLine::Version::kHttp11: ret += "HTTP/1.1"; break;
    default: ret += "UNKNOWN VERSION";
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
    std::scoped_lock guard{mutex_};
    connection_metadata_.erase(conn);
    return ;
  }
  auto num_read = msg.size();
  LOG_INFO("sockfd[%d] num_read[%d], contents:\n%s", conn->get_channel()->get_fd(), num_read, msg.c_str());
  if (metadata.parsing_state == HttpConnectionMetadata::ParsingState::kRequestLine) {
    auto pos_cr_lf = msg.find('\r');
    if (pos_cr_lf == std::string::npos) {
      metadata.in_buffer += std::move(msg);
      LOG_INFO("sockfd[%d] num_read[%u], wait for further read", conn->get_channel()->get_fd(), num_read);
      return ;
    }
    pos_cr_lf = metadata.in_buffer.size() + pos_cr_lf;
    metadata.in_buffer += std::move(msg);
    auto request_line = RequestLine::from(std::string_view{metadata.in_buffer.data(), pos_cr_lf + 1});
    if (!request_line) {
      // TODO: send error page and handle close.
      LOG_ERROR("Failed to parse request_line");
      return ;
    }
    LOG_INFO("Parsed request line[%s]", RequestLine::to_string(request_line.value()).c_str());
    metadata.request_line = request_line.value();
    metadata.parsing_state = HttpConnectionMetadata::ParsingState::kHeaderLines;
  }

  if (metadata.parsing_state == HttpConnectionMetadata::ParsingState::kHeaderLines) {
    metadata.parsing_state = HttpConnectionMetadata::ParsingState::kFinished;
    send_error_page(conn, 400, "Bad request");
    return ;
    int connection_fd = conn->get_channel()->get_fd();
    LOG_INFO("Request message(from[%d]):\n%sResponse:\n%s", 
        connection_fd, metadata.in_buffer.c_str(), kHelloWorld.data());
    if (::write(connection_fd, kHelloWorld.data(), kHelloWorld.size()) == -1) {
      LOG_ERROR("sockfd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
    }
    LOG_INFO("sockfd[%d] - Message responded", connection_fd);
  }
}

void HttpServer::send_error_page(const TcpConnectionPtr& conn, int error_code, std::string_view short_msg) const {
  std::string status = "HTTP/1.1 " + std::to_string(error_code) + " " + std::string{short_msg} + "\r\n";
  std::string_view headers{"Content-Type: text/html;charset=utf-8 \r\n\r\n"};
  std::string_view body = "<html><title>出错了~</title>";
  std::string error_contents = "<body><h1>ERROR<hr /></h1><p>" + 
    std::to_string(error_code) + ", " + std::string{short_msg} + "</p></body></html>";
  constexpr const int kNumIov{4};
  iovec iov[kNumIov];
  iov[0].iov_base = status.data();
  iov[0].iov_len = status.size();
  iov[1].iov_base = const_cast<void*>(reinterpret_cast<const void*>(headers.data()));
  iov[1].iov_len = headers.size();
  iov[2].iov_base = const_cast<void*>(reinterpret_cast<const void*>(body.data()));
  iov[2].iov_len = body.size();
  iov[3].iov_base = error_contents.data();
  iov[3].iov_len = error_contents.size();
  int connection_fd = conn->get_channel()->get_fd();
  LOG_INFO("sockfd[%d] - Message responding...", connection_fd);
  if (::writev(connection_fd, iov, kNumIov) == -1) {
    LOG_ERROR("sockfd[%d] failed to write [%d:%s]", connection_fd, errno, strerror_thread_local(errno));
  }
    LOG_INFO("sockfd[%d] - Message responded", connection_fd);
}

}
