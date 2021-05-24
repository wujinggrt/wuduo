#include <sstream>
#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <thread>
#include <chrono>

#include <sys/types.h>
#include <unistd.h>

#include "log.h"

void log_base(const char* fmt, ...) {
  char buf[512] = "";
  std::size_t i{0};
  std::va_list ap;

  auto now = std::chrono::system_clock::now();
  auto now_time = std::chrono::system_clock::to_time_t(now);
  std::ostringstream os;
  os << std::this_thread::get_id();
  i += std::snprintf(buf, sizeof(buf), "[%d, %s] %s ", 
      getpid(), os.str().c_str(), std::asctime(std::localtime(&now_time)));

  va_start(ap, fmt);
  i += std::vsnprintf(buf + i, sizeof(buf) - i, fmt, ap);
  va_end(ap);

  std::printf("%s", buf);
}
