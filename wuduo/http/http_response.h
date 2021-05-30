#pragma once

#include <unordered_map>
#include <string>
#include <string_view>

#include "wuduo/http/http_request.h"

namespace wuduo {

class Buffer;

}

namespace wuduo::http {

[[maybe_unused]] constexpr const std::string_view kHelloWorld =
  //"HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8 \r\n\r\nHello wOrld!";
  "Hello world";

[[maybe_unused]] constexpr const std::string_view kIndexPage =
  "<html><body><p>Index page</p><a href=\"./hello.html\">Hello world!</a></body></html>";

enum class StatusCode {
  k200Ok = 200,
  k301MovedPermanently = 301,
  k400BadRequest = 400,
  k404NotFound = 404,
  k505HttpVersionNotSupported = 505
};

std::string to_string(StatusCode code);

class HttpResponse {
 public:
  HttpResponse(bool close_connection = true)
    : status_code_{StatusCode::k404NotFound},
    phrase_{"Not Found"},
    close_connection_{close_connection}
  {
    set_close_connection(close_connection);
    set_content_type("text/html;charset=utf-8");
  }

  void set_status_code(StatusCode code) {
    set_phrase(to_string(code));
    status_code_ = code;
  }

  void set_phrase(std::string phrase) {
    phrase_ = std::move(phrase);
  }

  void add_header(std::string key, std::string value) {
    headers_[key] = std::move(value);
  }

  void set_content_type(std::string content_type) {
    add_header("Content-Type", std::move(content_type));
  }

  void set_entity_body(std::string body) {
    entity_body_ = std::move(body);
  }

  void set_error_page_to_entity_body();

  bool close_connection() const { return close_connection_; }
  void set_close_connection(bool close_connection) {
    add_header("Connection", close_connection ? "close" : "keep-alive");
    close_connection_ = close_connection; 
  }

  std::string response_message() const;
  
  void append_to(Buffer* output) const;

  void analyse(HttpRequest* request);

 private:
  std::unordered_map<std::string, std::string> headers_;
  StatusCode status_code_;
  std::string phrase_;
  std::string entity_body_;
  bool close_connection_;
};

}
