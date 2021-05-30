#pragma once

void log_base(const char* fmt, ...);

#define BASIC_LOG(level, fmt, ...) \
  log_base("\033[38;2;192;192;192m[%s:%d:%s] \033[0m" level " - " fmt "\n", __FILE__, __LINE__, __func__,  ##__VA_ARGS__)
  //std::printf("[%s:%d:%s]" " " level " - " fmt "\n", __FILE__, __LINE__, __func__,  ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...) BASIC_LOG("\033[38;2;255;0;0mFATAL\033[0m", fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) BASIC_LOG("\033[38;2;255;0;0mERROR\033[0m", fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) BASIC_LOG("\033[38;2;255;0;0mWARN\033[0m", fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) BASIC_LOG("\033[38;2;0;255;0mINFO\033[0m", fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) BASIC_LOG("DEBUG", fmt, ##__VA_ARGS__)

#define LOG_TRACE(fmt, ...) BASIC_LOG("TRACE", fmt, ##__VA_ARGS__)
