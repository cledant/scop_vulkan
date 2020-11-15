#include "ArgsParsing.hpp"
#include "EngineOptions.hpp"

#include <random>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <iostream>

static uint32_t
generate_seed()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dist;
    return (dist(gen));
}

void
displayHelp(EngineOptions &opts, std::string_view arg)
{
    (void)arg;
    (void)opts;
    std::cout << "./scop options:" << std::endl;
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
setSeed(EngineOptions &opts, std::string_view arg)
{
    try {
        int64_t seed = std::stol(arg.substr(7).data());
        if (seed < 0 || seed > std::numeric_limits<uint32_t>::max()) {
            throw std::runtime_error("");
        }
        opts.seed = seed;
    } catch (std::exception const &e) {
        std::stringstream err;
        err << "Seed should be a number between: "
            << std::numeric_limits<uint32_t>::min();
        err << " and " << std::numeric_limits<uint32_t>::max();
        throw std::runtime_error(err.str());
    }
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
    EngineOptions opts = {};
    opts.seed = generate_seed();
    static void (*func[NB_POSSIBLE_OPTIONS])(EngineOptions &,
                                             std::string_view) = {
        displayHelp, displayHelp, setInvertMouseY, setFullscreen
    };

    for (int i = 1; i < argc; ++i) {
        for (int y = 0; y < NB_POSSIBLE_OPTIONS; ++y) {
            if (!std::strncmp(
                  POSSIBLE_OPTIONS[y], argv[i], sizeof(POSSIBLE_OPTIONS[y]))) {
                (*func[y])(opts, argv[i]);
                break;
            }
        }
    }
    return (opts);
}