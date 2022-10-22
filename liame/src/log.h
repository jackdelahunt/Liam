#pragma once
#include "spdlog/spdlog.h"

extern spdlog::logger *engine_logger;
extern spdlog::logger *game_logger;

void init_logger();

#define LOG_INFO(format, ...) \
    engine_logger->info(format, __VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    engine_logger->debug(format, __VA_ARGS__)

#define LOG_ERR(format, ...) \
    engine_logger->error(format, __VA_ARGS__)

#define LOG_CRASH(format, ...) \
    engine_logger->info(format, __VA_ARGS__)

#define ASSERT(condition, format, ...) \
    if(!(condition)) { \
        engine_logger->error(format, __VA_ARGS__); \
        assert(0); \
    }
