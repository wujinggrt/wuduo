#include <cassert>

#include "wuduo/http/http_context.h"
#include "wuduo/log.h"

namespace wuduo::http {

bool HttpContext::parse_request(Buffer* buf) {
  if (buf->readable_bytes() == 0) {
    // wait further data.
    return true;
  }
  if (phase_ == ParsingPhase::kRequestLine) {
    std::string_view messages{buf->peek(), buf->readable_bytes()};
    const auto pos_cr_lf = messages.find("\r\n");
    if (pos_cr_lf == std::string_view::npos) {
      // wait further reading.
      return true;
    }
    if (!request_.set_request_line_from(
          std::string_view{messages.data(), pos_cr_lf})) {
      LOG_ERROR("request_line parse error");
      return false;
    }
    phase_ = ParsingPhase::kHeaderLines;
    assert(buf->readable_bytes() > (pos_cr_lf + 2));
    buf->retrieve(pos_cr_lf + 2);
    return parse_request(buf);
  } else if (phase_ == ParsingPhase::kHeaderLines) {
    std::string_view messages{buf->peek(), buf->readable_bytes()};
    const auto pos_cr_lf = messages.find("\r\n");
    if (pos_cr_lf == std::string_view::npos) {
      // wait further reading.
      return true;
    }

    std::string_view line{messages.data(), pos_cr_lf};
    if (line.empty()) {
      // blank line, only preceding \r\n
      // TODO: parse entity body.
      phase_ = ParsingPhase::kCompleted;
      return true;
    }
    if (!request_.add_header_from(line)) {
      return false;
    }

    assert(buf->readable_bytes() >= (pos_cr_lf + 2));
    buf->retrieve(pos_cr_lf + 2);
    return parse_request(buf);
  } else {
    // TODO: parse entity body.
  }
  return true;
}

}
