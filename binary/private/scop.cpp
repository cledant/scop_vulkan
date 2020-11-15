#include "ArgsParsing.hpp"
#include "Engine.hpp"

int
main(int argc, char const **argv)
{
    try {
        auto options = parseArgs(argc, argv);
        Engine engine;

        engine.init(options);
        engine.run();
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
    }
    return (0);
}