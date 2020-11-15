#ifndef SCOP_VULKAN_ARGSPARSING_HPP
#define SCOP_VULKAN_ARGSPARSING_HPP

#include "EngineOptions.hpp"

#include <string_view>

static int32_t const NB_POSSIBLE_OPTIONS = 5;
static char const POSSIBLE_OPTIONS[][32] = { "-h",
                                             "--help",
                                             "--invertMouseY",
                                             "--fullscreen" };

// General Parser
EngineOptions parseArgs(int32_t argc, const char **argv);

// Option Parser
void displayHelp(EngineOptions &opts, std::string_view arg);
void setInvertMouseY(EngineOptions &opts, std::string_view arg);
void setSeed(EngineOptions &opts, std::string_view arg);
void setFullscreen(EngineOptions &opts, std::string_view arg);

#endif // SCOP_VULKAN_ARGSPARSING_HPP
