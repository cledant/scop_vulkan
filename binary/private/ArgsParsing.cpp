#include "ArgsParsing.hpp"
#include "EngineOptions.hpp"

#include <cstring>
#include <iostream>

void
displayHelp(EngineOptions &opts, std::string_view arg)
{
    (void)arg;
    (void)opts;
    std::cout << "Usage: ./scop [Options...] [Model]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t --invertMouseY to invert mouse Y axis" << std::endl;
    std::cout << "\t --fullscreen to set fullscreen mode" << std::endl;
    std::cout << "\t -h | --help to display help" << std::endl;
    exit(0);
}

void
setInvertMouseY(EngineOptions &opts, std::string_view arg)
{
    (void)arg;
    opts.invert_y_axis = 1;
}

void
setFullscreen(EngineOptions &opts, std::string_view arg)
{
    (void)arg;
    opts.fullscreen = 1;
}

EngineOptions
parseArgs(int32_t argc, const char **argv)
{
    EngineOptions opts{};
    static void (*func[NB_POSSIBLE_OPTIONS])(EngineOptions &,
                                             std::string_view) = {
        displayHelp, displayHelp, setInvertMouseY, setFullscreen
    };

    for (int i = 1; i < (argc - 1); ++i) {
        for (int y = 0; y < NB_POSSIBLE_OPTIONS; ++y) {
            if (!std::strncmp(
                  POSSIBLE_OPTIONS[y], argv[i], sizeof(POSSIBLE_OPTIONS[y]))) {
                (*func[y])(opts, argv[i]);
                break;
            }
        }
    }
    opts.model_path = argv[argc - 1];
    return (opts);
}