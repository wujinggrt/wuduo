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
    if (!request_.set_request_line_from(
          std::string_view{in_buffer_.data(), pos_cr_lf})) {
      LOG_ERROR("request_line parse error");
      return false;
    }
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
    if (line.empty()) {
      // blank line, only preceding \r\n
      // TODO: parse entity body.
      phase_ = ParsingPhase::kCompleted;
      return true;
    }
    if (!request_.add_header_from(line)) {
      return false;
    }

    assert((pos_cr_lf + 2) <= in_buffer_.size());
    in_buffer_ = in_buffer_.substr(pos_cr_lf + 2);
    return parse_request(std::string{});
  } else {
    // TODO: parse entity body.
  }
  return true;
}

}
