#pragma once
#include "spdlog/spdlog.h"

extern spdlog::logger *engine_logger;
extern spdlog::logger *game_logger;

void init_logger();

#define LOG_INFO_ENGINE(format, ...) \
    engine_logger->info(format, __VA_ARGS__)

#define LOG_DEBUG_ENGINE(format, ...) \
    engine_logger->debug(format, __VA_ARGS__)

#define LOG_ERR_ENGINE(format, ...) \
    engine_logger->error(format, __VA_ARGS__)

#define LOG_CRASH_ENGINE(format, ...) \
    engine_logger->info(format, __VA_ARGS__)

#define ASSERT_ENGINE(condition, format, ...) \
    if(!(condition)) { \
        engine_logger->error(format, __VA_ARGS__); \
        assert(0); \
    }

#define LOG_INFO(format, ...) \
engine_logger->info(format, __VA_ARGS__)