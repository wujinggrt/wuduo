#pragma once

#include <algorithm>
#include <cstring>
#include <string_view>
#include <string>
#include <cassert>

namespace wuduo {

class Buffer {
 public:
  // for the convenience of prepending size to serialize.
  static constexpr const size_t kCheapPrepend = 8;
  static constexpr const size_t kInitialSize = 1024;
  static constexpr const char cr_lf[] = "\r\n";

  explicit Buffer(size_t initial_size = kInitialSize)
    : buffer_(kCheapPrepend + initial_size, '\0'),
    reader_index_{kCheapPrepend},
    writer_index_{kCheapPrepend}
  {
  }

  void swap(Buffer& other) {
    buffer_.swap(other.buffer_);
    std::swap(reader_index_, other.reader_index_);
    std::swap(writer_index_, other.writer_index_);
  }

  size_t readable_bytes() const {
    return writer_index_ - reader_index_;
  }

  size_t writable_bytes() const {
    return buffer_.size() - writer_index_;
  }

  // number of bytes that can be inserted into front of reader_index_.
  size_t prependable_bytes() const {
    return reader_index_;
  }

  // reveal to caller to read.
  const char* peek() const {
    return buffer_.data() + reader_index_;
  }

  const char* find_cr_lf() const {
    const char* pos_cr_lf = std::search(peek(), peek() + readable_bytes(), cr_lf, cr_lf + 2);
    return pos_cr_lf == peek() + readable_bytes() ? nullptr : pos_cr_lf;
  }

  void retrieve(size_t count) {
    assert(count <= readable_bytes());
    if (count == readable_bytes()) {
      retrieve_all();
    } else {
      reader_index_ += count;
    }
  }

  void retrieve_all() {
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
  }

  std::string retrieve_all_as_string() {
    std::string ret{peek(), readable_bytes()};
    retrieve_all();
    return ret;
  }

  void append(std::string_view str) {
    append(str.data(), str.size());
  }

  void append(const char* data, size_t count) {
    ensure_writable_bytes(count);
    std::copy_n(data, count, begin_write());
    has_written(count);
  }

  char* begin_write() { return &buffer_.front() + writer_index_; }
  const char* begin_write() const { return buffer_.data() + writer_index_; }

  void has_written(size_t count) {
    assert(count <= writable_bytes());
    writer_index_ += count;
  }

  void unwrite(size_t count) {
    assert(count <= readable_bytes());
    writer_index_ -= count;
  }

  void prepend(const char* data, size_t count) {
    assert(count <= prependable_bytes());
    reader_index_ -= count;
    std::copy_n(data, count, buffer_.data() + reader_index_);
  }

  void ensure_writable_bytes(size_t count) {
    if (writable_bytes() < count) {
      make_space(count);
    }
    assert(writable_bytes() >= count);
  }

  void shrink_to_fit() {
    buffer_.shrink_to_fit();
  }

  ssize_t read_fd(int fd, int* saved_errno = nullptr);

 private:
  void make_space(size_t count) {
    if (writable_bytes() + prependable_bytes() < count + kCheapPrepend) {
      buffer_.resize(writer_index_ + count);
      return ;
    }
    // case rotate.
    const size_t readable = readable_bytes();
    std::copy(buffer_.data() + reader_index_, 
              buffer_.data() + reader_index_ + readable,
              &buffer_.front() + kCheapPrepend);
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend + readable;
  }

  std::string buffer_;
  size_t reader_index_;
  size_t writer_index_;
};

}
