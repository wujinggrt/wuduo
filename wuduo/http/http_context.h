#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>

#include "wuduo/http/http_request.h"
#include "wuduo/buffer.h"

namespace wuduo::http {

class HttpContext {
 public:
  enum class ParsingPhase { kRequestLine, kHeaderLines, kEntityBody, kCompleted };
  enum class ParsingError { kCorrect, kRequestLine, kHeaderLine, kEntityBody, kUnknown };

  HttpContext()
    : phase_{ParsingPhase::kRequestLine}
  {}

  bool parse_request(Buffer* buf);

  bool parsing_completed() const {
    return phase_ == ParsingPhase::kCompleted; 
  }

  void reset() {
    using std::swap;
    phase_ = ParsingPhase::kRequestLine;
    HttpContext dummy;
    request_.swap(dummy.request_);
  }

  HttpRequest* request() { return &request_; }

 private:
  ParsingPhase phase_;
  HttpRequest request_;
};

}
