#include "cassert"

#include "wuduo/http/http_request.h"
#include "wuduo/log.h"

namespace wuduo::http {

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
    ret.method = Method::kGet;
  } else if (method == "HEAD") {
    ret.method = Method::kHead;
  } else if (method == "POST") {
    ret.method = Method::kPost;
  } else {
    return std::nullopt;
  }

  auto question_pos = url.find('?');
  if (question_pos != std::string_view::npos) {
    ret.path = std::string{url.data(), question_pos};
    ret.query = std::string{url.substr(question_pos + 1)};
  }

  ret.url = std::string{url};

  if (version == "HTTP/1.0") {
    ret.version = Version::kHttp10;
  } else if (version == "HTTP/1.1") {
    ret.version = Version::kHttp11;
  } else {
    return std::nullopt;
  }

  return ret;
}

std::string RequestLine::to_string(const RequestLine& line) {
  std::string ret;

  switch (line.method) {
    case Method::kGet: ret += "GET"; break;
    case Method::kHead: ret += "HEAD"; break;
    case Method::kPost: ret += "POST"; break;
    default: ret += "UNKNOWN"; break;
  }

  ret += " " + line.url + " ";

  switch (line.version) {
    case Version::kHttp10: ret += "HTTP/1.0"; break;
    case Version::kHttp11: ret += "HTTP/1.1"; break;
    default: ret += "UNKNOWN VERSION";
  }

  return ret;
}

bool HttpRequest::set_request_line_from(std::string_view line) {
  auto request_line = RequestLine::from(line);
  if (request_line.has_value()) {
    request_line_ = std::move(request_line.value());
    return true;
  }
  return false;
}

bool HttpRequest::add_header_from(std::string_view line) {
  const auto pos_colon = line.find(':');
  if (pos_colon == std::string_view::npos) {
    LOG_ERROR("failed to parse header's value");
    return false;
  }
  const auto pos_value = line.find_first_not_of(' ', pos_colon + 1);
  if (pos_value == std::string_view::npos) {
    // may matche next line.
    LOG_ERROR("failed to parse header's value");
    return false;
  }
  std::string key{line.substr(0, pos_colon)};
  std::string value{line.substr(pos_value)};
  LOG_DEBUG("parsed header: [%s, %s]", key.c_str(), value.c_str());
  return headers_.emplace(std::move(key), std::move(value)).second;
}

}
