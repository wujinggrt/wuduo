#pragma once

#include <string_view>
#include <string>
#include <optional>

[[maybe_unused]] constexpr const char kHelloWorld[] =
  "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8 \r\n\r\nHello wOrld!";

[[maybe_unused]] constexpr const char kBadRequest[] =
  "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html;charset=utf-8 \r\n\r\n";

struct RequestLine {
  std::string method;
  std::string url;
  std::string version;

  static std::optional<RequestLine> from(std::string_view line);
};
