#include <cassert>

#include "wuduo/http/http_context.h"
#include "wuduo/log.h"

namespace wuduo::http {

bool HttpContext::parse_request(std::string msg) {
  if (!msg.empty()) {
    in_buffer_ += std::move(msg);
  }
  if (phase_ == ParsingPhase::kRequestLine) {
    const auto pos_cr_lf = in_buffer_.find("\r\n");
    if (pos_cr_lf == std::string::npos) {
      // wait further reading.
      return true;
    }
    auto request_line = RequestLine::from(
      std::string_view{in_buffer_.data(), pos_cr_lf});
    if (!request_line.has_value()) {
      LOG_ERROR("request_line parse error");
      return false;
    }
    request_line_ = std::move(request_line.value());
    phase_ = ParsingPhase::kHeaderLines;
    // remove the request line and the suffix, \r\n.
    assert((pos_cr_lf + 2) <= in_buffer_.size());
    in_buffer_ = in_buffer_.substr(pos_cr_lf + 2);
    return parse_request(std::string{});
  } else if (phase_ == ParsingPhase::kHeaderLines) {
    const auto pos_cr_lf = in_buffer_.find("\r\n");
    if (pos_cr_lf == std::string::npos) {
      // wait further reading.
      return true;
    }

    std::string_view line{in_buffer_.data(), pos_cr_lf};
    const auto pos_colon = line.find(':');
    if (pos_colon == std::string_view::npos) {
      // blank line, only preceding \r\n
      // TODO: parse entity body.
      phase_ = ParsingPhase::kCompleted;
      return true;
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
    headers_.emplace(std::move(key), std::move(value));

    assert((pos_cr_lf + 2) <= in_buffer_.size());
    in_buffer_ = in_buffer_.substr(pos_cr_lf + 2);
    return parse_request(std::string{});
  } else {
    // TODO: parse entity body.
  }
  return true;
}

bool HttpContext::add_header(std::string_view line) {
  auto pos_colon = line.find(':');
  if (pos_colon == std::string_view::npos) {
    // blank line.
    return false;
  }
  return true;
}

}
