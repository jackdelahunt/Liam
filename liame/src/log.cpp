#include "log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

spdlog::logger *engine_logger = NULL;
spdlog::logger *game_logger = NULL;


void init_logger()
{
    spdlog::set_pattern("%^[%T] :: %n :: %l :: %v%$");

    auto console = 

    engine_logger = spdlog::stdout_color_mt("ENGINE").get(); 
    engine_logger->set_level(spdlog::level::trace);

    game_logger = spdlog::stdout_color_mt("GAME").get(); 
    game_logger->set_level(spdlog::level::trace);
}