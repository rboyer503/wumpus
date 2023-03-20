#include "Game.h"

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdexcept>


Game::Game() :
    myTerminal(std::make_unique<OSTerminal>()),
    myActiveWorld(myTerminal.get())
{
}

void Game::initialize()
{
    if (!myTerminal->initialize())
        throw std::runtime_error("Terminal initialization failed");

    if (!myTerminal->setMode(eTermMode::TM_GAME))
        throw std::runtime_error("Terminal setMode failed");
}

void Game::executiveLoop()
{
    while (!myExiting)
    {
        switch (myGameState)
        {
        case GameState::init:
            processInit();
            break;

        case GameState::game:
            processGame();
            break;

        case GameState::gameover:
            processGameOver();
            break;

        default:
            throw std::runtime_error("Unexpected game state");
        }

        boost::this_thread::sleep(boost::posix_time::millisec(10));
    }
}

void Game::processInit()
{
    myActiveWorld.render();

    myGameState = GameState::game;
}

void Game::processGame()
{
    kb_codes_vec kbCodes;

    if (myTerminal->pollKeys(kbCodes))
    {
        switch (kbCodes[0])
        {
        case KB_UP:
            myActiveWorld.moveSelection(World::MoveDirection::up);
            break;

        case KB_DOWN:
            myActiveWorld.moveSelection(World::MoveDirection::down);
            break;

        case KB_LEFT:
            myActiveWorld.moveSelection(World::MoveDirection::left);
            break;

        case KB_RIGHT:
            myActiveWorld.moveSelection(World::MoveDirection::right);
            break;

        case KB_SPACE:
        case KB_ENTER:
            myActiveWorld.move();
            break;

        case KB_W:
            myActiveWorld.toggleWumpus();
            break;

        case KB_U:
            myActiveWorld.toggleUnknown();
            break;

        case KB_Q:
        case KB_ESCAPE:
            myExiting = true;
            break;
        }
    }

    if (myActiveWorld.isGameOver())
    {
        myGameState = GameState::gameover;
    }
}

void Game::processGameOver()
{
    kb_codes_vec kbCodes;

    if (myTerminal->pollKeys(kbCodes))
    {
        switch (kbCodes[0])
        {
        case KB_UP:
        case KB_DOWN:
        case KB_LEFT:
        case KB_RIGHT:
            // Ignore arrow keys to minimize chance user exits without noticing the game over message.
            break;

        default:
            myExiting = true;
            break;
        }
    }
}
