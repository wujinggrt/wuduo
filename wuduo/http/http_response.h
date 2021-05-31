#pragma once

#include <memory>
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

class MimeType {
 public:
  static std::string from(std::string suffix);

  static constexpr const std::string_view kDefault{"text/html;charset=utf-8"};

 private:
  MimeType();

  std::unordered_map<std::string, std::string> types_;
};

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
  HttpResponse(bool close_connection = true);

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

  void set_entity_body(std::string_view body);

  void set_error_page_with(StatusCode code);

  void set_error_page_to_entity_body();

  bool close_connection() const { return close_connection_; }
  void set_close_connection(bool close_connection) {
    add_header("Connection", close_connection ? "close" : "keep-alive");
    close_connection_ = close_connection; 
  }

  std::string response_message() const;
  // avoid copy.
  Buffer* response_message_as_internal_buffer() const { return response_messages_.get(); }
  
  void append_to(Buffer* output) const;

  Buffer* analyse_and_get_response_message_buffer();
  void analyse(HttpRequest* request);

 private:
  void append_status_line_and_headers_to(Buffer* output) const;

  std::unordered_map<std::string, std::string> headers_;
  StatusCode status_code_;
  std::string phrase_;
  std::unique_ptr<Buffer> entity_body_;
  std::unique_ptr<Buffer> response_messages_;
  bool close_connection_;
};

}
