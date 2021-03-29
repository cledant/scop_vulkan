#include "Engine.hpp"

int
main()
{
    static constexpr char const *APP_NAME = "scop";

    try {
        Engine engine;

        engine.init(APP_NAME);
        engine.run();
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
    }
    return (0);
}