#pragma once

#include <string_view>
#include <string>
#include <optional>
#include <unordered_map>

namespace wuduo::http {

enum class Method { kGet, kHead, kPost, kUnknown };
enum class Version { kHttp10, kHttp11, kUnknown };

class RequestLine {
 public:

  RequestLine() = default;

  Method method{Method::kUnknown};
  std::string url;
  std::string path;
  std::string query;
  Version version{Version::kUnknown};

  static std::optional<RequestLine> from(std::string_view line);

  static std::optional<RequestLine> from(std::string_view method, 
                                         std::string_view url, 
                                         std::string_view version);
  static std::string to_string(const RequestLine& line);
};

class HttpRequest {
 public:
  HttpRequest() = default;

  Method method() const { return request_line_.method; }
  Version version() const { return request_line_.version; }
  std::string path() const { return request_line_.path; }
  std::string query() const { return request_line_.query; }
  std::string url() const { return request_line_.url; }
  
  bool set_request_line_from(std::string_view line);

  bool add_header_from(std::string_view line);

  std::optional<std::string> get_header(const std::string& key) const {
    const auto it = headers_.find(key);
    if (it != headers_.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  void swap(HttpRequest& other) {
    using std::swap;
    swap(request_line_, other.request_line_);
    swap(headers_, other.headers_);
  }

  bool is_close_connection() const;

 private:
  RequestLine request_line_;
  std::unordered_map<std::string, std::string> headers_;
};

}
