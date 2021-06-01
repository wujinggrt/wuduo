#include <cstring>
#include <unordered_map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wuduo/http/http_response.h"
#include "wuduo/buffer.h"
#include "wuduo/log.h"

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
  types_[".mp4"] = "video/mp4";
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

HttpResponse::HttpResponse(bool close_connection)
  : status_code_{StatusCode::k404NotFound},
  phrase_{"Not Found"},
  entity_body_{std::make_unique<Buffer>()},
  response_messages_{std::make_unique<Buffer>()},
  close_connection_{close_connection}
{
  set_close_connection(close_connection);
  set_content_type("text/html;charset=utf-8");
}

void HttpResponse::set_entity_body(std::string_view body) {
  entity_body_->append(body);
}

std::unique_ptr<Buffer> HttpResponse::error_message_with(StatusCode code) {
  set_status_code(code);
  set_content_type(std::string{MimeType::kDefault});
  entity_body_->retrieve_all();
  entity_body_->append("<html><title>出错了~</title>");
  entity_body_->append("<body><h1>ERROR<hr /></h1><p>");
  const std::string error_code = std::to_string(static_cast<int>(code));
  entity_body_->append(std::string_view{error_code});
  entity_body_->append(", ");
  entity_body_->append(phrase_);
  entity_body_->append("</p></body></html>");
  auto buffer = std::make_unique<Buffer>();
  append_status_line_and_headers_to(buffer.get());
  buffer->append(entity_body_->peek(), entity_body_->readable_bytes());
  return buffer;
}

void HttpResponse::append_to(Buffer* output) const {
  append_status_line_and_headers_to(output);
  output->append(entity_body_->peek(), entity_body_->readable_bytes());
}

std::unique_ptr<Buffer> HttpResponse::analyse(HttpRequest* request) {
  auto output = std::make_unique<Buffer>();
  set_close_connection(request->is_close_connection());
  const auto pos_dot = request->path().find_last_of('.');
  if (pos_dot == std::string::npos) {
    set_content_type(std::string{MimeType::kDefault});
    LOG_INFO("responsed type unrecognized, as default");
  } else {
    set_content_type(
        MimeType::from(request->path().substr(pos_dot)));
    auto mime_type = MimeType::from(request->path().substr(pos_dot));
    LOG_INFO("responsed type[%.*s:%s]", 
                                        request->path().size() - pos_dot, 
                                        request->path().data() + pos_dot, 
                                        mime_type.c_str());
  }

  auto fd = ::open(request->path().c_str(), O_RDONLY | O_CLOEXEC);
  if (fd == -1) {
    LOG_ERROR("file not found [%s]", request->path().c_str());
    return error_message_with(StatusCode::k404NotFound);
  }
  struct ::stat file_metadata;
  if (::fstat(fd, &file_metadata) == -1) {
    ::close(fd);
    LOG_ERROR("failed to fetch file metadata [%s]", request->path().c_str());
    return error_message_with(StatusCode::k404NotFound);
  }
  LOG_INFO("File size [%s:%ld]", request->path().c_str(), file_metadata.st_size);
  // reads until file completed buffered.
  for (;;) {
    auto num_read = entity_body_->read_fd(fd);
    if (num_read < 0) {
      // process error not found.
      ::close(fd);
      return error_message_with(StatusCode::k404NotFound);
    } else if (entity_body_->readable_bytes() < static_cast<size_t>(file_metadata.st_size)) {
      continue;
    } else {
      assert(entity_body_->readable_bytes() == static_cast<size_t>(file_metadata.st_size));
      ::close(fd);
      append_status_line_and_headers_to(output.get());
      output->append(entity_body_->peek(), entity_body_->readable_bytes());
      return output;
    }
  }
}

void HttpResponse::append_status_line_and_headers_to(Buffer* output) const {
  char buf[32];
  auto num = std::snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", static_cast<int>(status_code_));
  if (num < 0) { 
    return ;
  }
  output->append(std::string_view(buf, num));
  output->append(phrase_);
  output->append("\r\n");

  num = snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", entity_body_->readable_bytes());
  if (num >= 0) {
    output->append(std::string_view(buf, num));
  }

  for (const auto& [k, v] : headers_) {
    output->append(k);
    output->append(": ");
    output->append(v);
    output->append("\r\n");
  }
  output->append("\r\n");
}

}
