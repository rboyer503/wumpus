#include "Game.h"
#include <iostream>
#include <stdexcept>


int main()
{
    try
    {
        Game game;
        game.initialize();
        game.executiveLoop();
    }
    catch (std::runtime_error & e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
