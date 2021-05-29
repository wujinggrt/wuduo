#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "wuduo/http/http_request.h"

namespace wuduo::http {

class HttpContext {
 public:
  enum class ParsingPhase { kRequestLine, kHeaderLines, kEntityBody, kCompleted };
  enum class ParsingError { kCorrect, kRequestLine, kHeaderLine, kEntityBody, kUnknown };

  HttpContext()
    : phase_{ParsingPhase::kRequestLine}
  {}

  bool parse_request(std::string msg);

  bool is_parsing_completed() const {
    return phase_ == ParsingPhase::kCompleted; 
  }

  void reset() {
    phase_ = ParsingPhase::kRequestLine;
    HttpContext dummy;
    std::swap(in_buffer_, dummy.in_buffer_);
    std::swap(out_buffer_, dummy.out_buffer_);
    std::swap(request_line_, dummy.request_line_);
  }

  bool add_header(std::string_view line);

 private:
  ParsingPhase phase_;
  std::string in_buffer_;
  std::string out_buffer_;
  RequestLine request_line_;
  std::unordered_map<std::string, std::string> headers_;
};

}
