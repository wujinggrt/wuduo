#pragma once

#include <iostream>
#include <cstdio>

#define BASIC_LOG(level, fmt, ...) \
  std::printf("[" level "] " fmt "\n", ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...) BASIC_LOG("FATAL", fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) BASIC_LOG("ERROR", fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) BASIC_LOG("WARN", fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) BASIC_LOG("INFO", fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) BASIC_LOG("DEBUG", fmt, ##__VA_ARGS__)
