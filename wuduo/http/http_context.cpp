#include <cassert>

#include "wuduo/http/http_context.h"
#include "wuduo/log.h"

namespace wuduo::http {

bool HttpContext::parse_request(Buffer* buf) {
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
      assert(pos_cr_lf == 0);
      phase_ = ParsingPhase::kEntityBody;
      buf->retrieve(2);
      return parse_request(buf);
    }
    if (!request_.add_header_from(line)) {
      return false;
    }

    assert(buf->readable_bytes() >= (pos_cr_lf + 2));
    buf->retrieve(pos_cr_lf + 2);
    return parse_request(buf);
  } else if (phase_ == ParsingPhase::kEntityBody) {
    // TODO: parse entity body.
    if (request_.method() != Method::kPost) {
      phase_ = ParsingPhase::kCompleted;
      return true;
    }
    auto contents_length = request_.get_header("Content-length");
    if (!contents_length.has_value()) {
      // bad request.
      return false;
    }
    long long contents_length_value = -1;
    try {
      contents_length_value = std::stoll(contents_length.value());
    } catch (...) {

    }
    if (contents_length_value < 0) {
      return false;
    }
    if (buf->readable_bytes() < static_cast<size_t>(contents_length_value)) {
      return true;
    }
    buf->retrieve_all();
    phase_ = ParsingPhase::kCompleted;
    return true;
  }
  return true;
}

}
