#pragma once

#include <string_view>
#include <string>
#include <optional>

namespace wuduo::http {


class RequestLine {
 public:
  enum class Method { kGet, kHead, kPost, kUnknown };
  enum class Version { kHttp10, kHttp11, kUnknown };

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


}
