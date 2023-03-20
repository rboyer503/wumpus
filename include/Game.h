#ifndef GAME_H
#define GAME_H

#include "OSTerminal.h"
#include "World.h"
#include <memory>


// Class managing the game play.
class Game
{
public:
    enum class GameState
    {
        init,
        game,
        gameover,
        MAX
    };

    Game();

    void initialize();
    void executiveLoop();

private:
    void processInit();
    void processGame();
    void processGameOver();

    std::unique_ptr<ITerminal> myTerminal;
    World myActiveWorld;
    bool myExiting = false;
    GameState myGameState = GameState::init;
};

#endif // GAME_H
