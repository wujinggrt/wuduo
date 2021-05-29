#include "cassert"

#include "http_request.h"

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
    ret.method = RequestLine::Method::kGet;
  } else if (method == "HEAD") {
    ret.method = RequestLine::Method::kHead;
  } else if (method == "POST") {
    ret.method = RequestLine::Method::kPost;
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

}
