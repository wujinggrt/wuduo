#include <cstring>
#include <unordered_map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wuduo/http/http_response.h"
#include "wuduo/buffer.h"

namespace wuduo::http {

std::string MimeType::from(std::string suffix) {
  static MimeType mime;
  auto iter = mime.types_.find(suffix);
  return iter == mime.types_.end() ? mime.types_["default"] : iter->second;
}

MimeType::MimeType() {
  types_[".html"] = std::string{kDefault};
  types_[".avi"] = "video/x-msvideo";
  types_[".bmp"] = "image/bmp";
  types_[".c"] = "text/plain";
  types_[".doc"] = "application/msword";
  types_[".gif"] = "image/gif";
  types_[".gz"] = "application/x-gzip";
  types_[".htm"] = "text/html";
  types_[".ico"] = "image/x-icon";
  types_[".jpg"] = "image/jpeg";
  types_[".png"] = "image/png";
  types_[".txt"] = "text/plain";
  types_[".mp3"] = "audio/mp3";
  types_["default"] = std::string{kDefault};
}

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

void HttpResponse::set_error_page_with(StatusCode code) {
  set_status_code(code);
  set_content_type(std::string{MimeType::kDefault});
  set_error_page_to_entity_body();
}

void HttpResponse::set_error_page_to_entity_body() {
  Buffer buf;
  buf.append("<html><title>出错了~</title>");
  buf.append("<body><h1>ERROR<hr /></h1><p>");
  std::string error_code = std::to_string(static_cast<int>(status_code_));
  buf.append(std::string_view{error_code});
  buf.append(", ");
  buf.append(phrase_);
  buf.append("</p></body></html>");

  set_entity_body(std::string{buf.peek(), buf.readable_bytes()});
  buf.retrieve_all();
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

  if (!close_connection_) {
    num = snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", entity_body_.size());
    if (num >= 0) {
      output->append(std::string_view(buf, num));
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

void HttpResponse::analyse(HttpRequest* request) {
  set_close_connection(request->is_close_connection());
  if (request->path() == "index.html") {
    set_entity_body(std::string{kIndexPage});
    return ;
  }

  const auto pos_dot = request->path().find_last_of('.');

  if (pos_dot == std::string::npos) {
    set_content_type(std::string{MimeType::kDefault});
  } else {
    set_content_type(
        MimeType::from(request->path().substr(pos_dot)));
  }

  auto fd = ::open(request->path().c_str(), O_RDONLY | O_CLOEXEC);
  if (fd == -1) {
    set_error_page_with(StatusCode::k404NotFound);
    return ;
  }
  Buffer buf;
  auto num_read = buf.read_fd(fd);
  ::close(fd);
  if (num_read < 0) {
    set_error_page_with(StatusCode::k404NotFound);
    return ;
  }
  set_entity_body(std::string{buf.peek(), buf.readable_bytes()});
}

}
