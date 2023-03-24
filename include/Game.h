#ifndef GAME_H
#define GAME_H

#include "OSTerminal.h"
#include "World.h"
#include <memory>


namespace GameMessage
{
    enum
    {
        CLEAR,
        START,
        MAX
    };
}


// Class managing the game play.
class Game
{
public:
    enum class GameState
    {
        splash,
        game,
        gameover,
        MAX
    };

    Game();

    void initialize();
    void executiveLoop();

private:
    static const std::string myBanner;
    static const std::string myMessages[GameMessage::MAX];

    void updateState(const GameState gameState);
    void processSplash();
    void processGame();
    void processGameOver();

    std::unique_ptr<ITerminal> myTerminal;
    World myActiveWorld;
    bool myExiting = false;
    GameState myGameState = GameState::splash;
    bool myStateInit = true;
};

#endif // GAME_H
