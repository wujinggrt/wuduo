#include <sys/uio.h>
#include <error.h>
#include <unistd.h>

#include "wuduo/buffer.h"

namespace wuduo {

ssize_t Buffer::read_fd(int fd, int* saved_errno) {
  char extrabuf[65535];
  iovec vec[2];
  const auto writable = writable_bytes();
  vec[0].iov_base = begin_write();
  vec[0].iov_len = writable_bytes();
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const auto n = ::readv(fd, vec, 2);
  if (n < 0) {
    if (saved_errno != nullptr) {
      *saved_errno = errno;
    }
  } else if (static_cast<size_t>(n) <= writable) {
    has_written(n);
  } else {
    has_written(writable);
    append(extrabuf, n - writable);
  }
  return n;
}

}
