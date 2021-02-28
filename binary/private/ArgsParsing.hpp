#ifndef SCOP_VULKAN_ARGSPARSING_HPP
#define SCOP_VULKAN_ARGSPARSING_HPP

#include "EngineOptions.hpp"

#include <string_view>

enum ARGS_OPTIONS
{
    AO_HELP_MINI = 0,
    AO_HELP_FULL,
    AO_INVERT_MOUSE_Y,
    AO_FULLSCREEN,
    AO_SCALE,
    AO_TOTAL_OPTION,
};

static char const POSSIBLE_OPTIONS[][32] = { "-h",
                                             "--help",
                                             "--invertMouseY",
                                             "--fullscreen",
                                             "--scale=" };

// General Parser
EngineOptions parseArgs(int32_t argc, const char **argv);

// Option Parser
void setInvertMouseY(EngineOptions &opts, std::string_view arg);
void setSeed(EngineOptions &opts, std::string_view arg);
void setFullscreen(EngineOptions &opts, std::string_view arg);
void setScale(EngineOptions &opts, std::string_view arg);
void setDisplayHelpMini(EngineOptions &opts, std::string_view arg);
void setDisplayHelpFull(EngineOptions &opts, std::string_view arg);

// Help
void displayHelp();

#endif // SCOP_VULKAN_ARGSPARSING_HPP
