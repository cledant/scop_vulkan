#include "ArgsParsing.hpp"
#include "EngineOptions.hpp"

#include <cstring>
#include <sstream>
#include <iostream>

EngineOptions
parseArgs(int32_t argc, const char **argv)
{
    EngineOptions opts{};
    static void (*func[AO_TOTAL_OPTION])(
      EngineOptions &, std::string_view) = { setDisplayHelpMini,
                                             setDisplayHelpFull,
                                             setInvertMouseY,
                                             setFullscreen,
                                             setScale };

    for (int32_t i = 1; i < (argc - 1); ++i) {
        for (int32_t y = 0; y < AO_TOTAL_OPTION; ++y) {
            if (!std::strncmp(POSSIBLE_OPTIONS[y],
                              argv[i],
                              std::strlen(POSSIBLE_OPTIONS[y]))) {
                (*func[y])(opts, argv[i]);
                break;
            }
        }
    }
    opts.model_path = argv[argc - 1];
    return (opts);
}

void
setInvertMouseY(EngineOptions &opts, std::string_view arg)
{
    if (arg.size() != std::strlen(POSSIBLE_OPTIONS[AO_INVERT_MOUSE_Y])) {
        return;
    }
    opts.invert_y_axis = 1;
}

void
setFullscreen(EngineOptions &opts, std::string_view arg)
{
    if (arg.size() != std::strlen(POSSIBLE_OPTIONS[AO_FULLSCREEN])) {
        return;
    }
    opts.fullscreen = 1;
}

void
setScale(EngineOptions &opts, std::string_view arg)
{
    try {
        float scale =
          std::stof(arg.substr(std::strlen(POSSIBLE_OPTIONS[AO_SCALE])).data());
        if (scale < MINIMAL_MODEL_SCALE ||
            scale > std::numeric_limits<float>::max()) {
            throw std::runtime_error("");
        }
        opts.scale = scale;
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
        std::stringstream err;
        err << "Scale size must be superior to " << MINIMAL_MODEL_SCALE;
        throw std::runtime_error(err.str());
    }
}

void
setDisplayHelpMini(EngineOptions &opts, std::string_view arg)
{
    (void)opts;
    if (arg.size() != std::strlen(POSSIBLE_OPTIONS[AO_HELP_MINI])) {
        return;
    }
    displayHelp();
}

void
setDisplayHelpFull(EngineOptions &opts, std::string_view arg)
{
    (void)opts;
    if (arg.size() != std::strlen(POSSIBLE_OPTIONS[AO_HELP_FULL])) {
        return;
    }
    displayHelp();
}

void
displayHelp()
{
    std::cout << "Usage: ./scop [Options...] [Model]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t --invertMouseY to invert mouse Y axis" << std::endl;
    std::cout << "\t --fullscreen to set fullscreen mode" << std::endl;
    std::cout << "\t --scale [Value] to set model scaling" << std::endl;
    std::cout << "\t -h | --help to display help" << std::endl;
    exit(0);
}