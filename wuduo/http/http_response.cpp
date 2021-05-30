#include <cstring>

#include "wuduo/http/http_response.h"
#include "wuduo/buffer.h"

namespace wuduo::http {

std::string to_string(StatusCode code) {
  switch (code) {
    case StatusCode::k200Ok: return "OK";
    case StatusCode::k301MovedPermanently: return "Moved Permanentyly";
    case StatusCode::k400BadRequest: return "Bad Request";
    case StatusCode::k404NotFound: return "Not Found";
    case StatusCode::k505HttpVersionNotSupported: return "HTTP Version Not Supported";
  }
  return "Unknown";
}

std::string HttpResponse::response_message() const {
  std::string ret;
  char buf[32];
  auto num = std::snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", static_cast<int>(status_code_));
  if (num < 0) {
    return "";
  }
  std::string cr_lf{"\r\n"};
  std::string status_line = std::string(buf, num) + phrase_ + cr_lf;
  std::string header_lines;
  for (const auto& [k, v] : headers_) {
    header_lines += k + std::string{": "} + v + cr_lf;
  }
  header_lines += cr_lf;
  return status_line + header_lines + entity_body_;
}

void HttpResponse::append_to(Buffer* output) const {
  char buf[32];
  auto num = std::snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", static_cast<int>(status_code_));
  if (num < 0) { 
    return ;
  }
  output->append(std::string_view(buf, num));
  output->append(phrase_);
  output->append("\r\n");

  if (close_connection_)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    num = snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", entity_body_.size());
    if (num >= 0) {
      output->append(std::string_view(buf, num));
      output->append("Connection: Keep-Alive\r\n");
    }
  }

  for (const auto& [k, v] : headers_) {
    output->append(k);
    output->append(": ");
    output->append(v);
    output->append("\r\n");
  }

  output->append("\r\n");
  output->append(entity_body_);
}

}
