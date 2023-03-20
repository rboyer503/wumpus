#include "World.h"

#include "OSTerminal.h"
#include <cmath>
#include <iostream>


const room_data_t World::myDefaultRoomData[] = {
    0,          0,          RP_VALID,           RP_VALID|RP_TREASURE,   RP_VALID,   0,                  0,
    0,          RP_VALID,   RP_VALID|RP_WUMPUS, RP_VALID,               RP_VALID,   RP_VALID,           0,
    RP_VALID,   RP_VALID,   RP_VALID,           RP_VALID,               RP_VALID,   RP_VALID|RP_WUMPUS, RP_VALID,
    RP_VALID,   RP_VALID,   RP_VALID,           RP_VALID,               RP_VALID,   RP_VALID,           RP_VALID,
    0,          RP_VALID,   RP_VALID,           RP_VALID,               RP_VALID,   RP_VALID,           0,
    0,          0,          RP_VALID,           RP_VALID,               RP_VALID,   0,                  0
};

const World::RawData World::myDefaultRawData = {
    7, 6, 3, 5, myDefaultRoomData
};

// Indexed as bDCBA, where:
//  A = RA_TOPLEFT
//  B = RA_TOPRIGHT
//  C = RA_BOTTOMLEFT
//  D = RA_BOTTOMRIGHT
const std::string World::myCornerStyles[][2] = {
    {" ", " "},
    {"┘", "╝"},
    {"└", "╚"},
    {"┴", "╩"},
    {"┐", "╗"},
    {"┤", "╣"},
    {"┼", "╬"},
    {"┼", "╬"},
    {"┌", "╔"},
    {"┼", "╬"},
    {"├", "╠"},
    {"┼", "╬"},
    {"┬", "╦"},
    {"┼", "╬"},
    {"┼", "╬"},
    {"┼", "╬"}
};

const std::string World::myLineStyles[][2] = {
    {"───", "═══"},
    {"│ ", "║ "},
    {" │", " ║"}
};

const std::string World::mySpecialSymbols[] = {
    "ʘ", // "😎",
    "ω", // "👹",
    "🔑",
    "▣", // "🔒",
    "?" // "❓️"
};

const std::string World::myMessages[] = {
    "                                                                                ",
    "Sorry, you can only move 1 space at a time.                                     ",
    "You hear a wumpus lurking nearby...                                             ",
    "AAAACK! You've been eaten by a wumpus!                                          ",
    "You've found the treasure - you win!                                            ",
    "--- Press a key to exit ---                                                     "
};


World::World(ITerminal * terminal) :
    myTerminal(terminal)
{
    load(myDefaultRawData);
}

void World::load(const RawData & rawData)
{
    myWidth = rawData.width;
    myHeight = rawData.height;
    mySelectX = myCurrX = rawData.startX;
    mySelectY = myCurrY = rawData.startY;

    // Allocate contiguous block that can be accessed via 2D array.
    myRoomData = std::make_unique<room_data_t[]>(myWidth * myHeight);
    myRoomGrid = std::make_unique<room_data_t*[]>(myWidth);

    for (int y = 0; y < myHeight; ++y)
    {
        myRoomGrid[y] = &myRoomData[y * myWidth];
    }

    // Now copy the raw data in.
    std::memcpy(myRoomData.get(), rawData.data, myWidth * myHeight * sizeof(room_data_t));
}

void World::render() const
{
    for (int y = 0; y < myHeight; ++y)
    {
        for (int x = 0; x < myWidth; ++x)
        {
            if (myRoomGrid[y][x] & RP_VALID)
            {
                renderRoom(x, y);
            }
        }
    }

    // Render selected room again to ensure double lines are "on top".
    renderSelectedRoom();

    myTerminal->doRefresh();
}

void World::renderRoom(int x, int y) const
{
    updateKernel(x, y);

    int xOffset = x * 4;
    int yOffset = y * 2;
    int drawStyle = DS_SINGLE;

    if ((x == mySelectX) && (y == mySelectY))
    {
        drawStyle = DS_DOUBLE;
    }

    std::ostringstream ossTop;
    ossTop << getCornerStyle(0, 0, drawStyle) << myLineStyles[LS_HORIZONTAL][drawStyle] <<
        getCornerStyle(1, 0, drawStyle);
    myTerminal->setCursorPos(xOffset, yOffset);
    myTerminal->output(ossTop, false);

    std::ostringstream ossMiddle;
    ossMiddle << myLineStyles[LS_LEFT_VERT][drawStyle] << getRoomContent(x, y) <<
        myLineStyles[LS_RIGHT_VERT][drawStyle];
    myTerminal->setCursorPos(xOffset, yOffset + 1);
    myTerminal->output(ossMiddle, false);

    std::ostringstream ossBottom;
    ossBottom << getCornerStyle(0, 1, drawStyle) << myLineStyles[LS_HORIZONTAL][drawStyle] <<
        getCornerStyle(1, 1, drawStyle);
    myTerminal->setCursorPos(xOffset, yOffset + 2);
    myTerminal->output(ossBottom, false);
}

void World::renderSelectedRoom() const
{
    renderRoom(mySelectX, mySelectY);
}

void World::moveSelection(MoveDirection direction)
{
    bool dirty = false;
    int oldX = mySelectX;
    int oldY = mySelectY;

    switch (direction)
    {
    case MoveDirection::up:
        if (mySelectY > 0)
        {
            if (myRoomGrid[mySelectY - 1][mySelectX] & RP_VALID)
            {
                --mySelectY;
                dirty = true;
            }
        }
        break;

    case MoveDirection::down:
        if (mySelectY < myHeight - 1)
        {
            if (myRoomGrid[mySelectY + 1][mySelectX] & RP_VALID)
            {
                ++mySelectY;
                dirty = true;
            }
        }
        break;

    case MoveDirection::left:
        if (mySelectX > 0)
        {
            if (myRoomGrid[mySelectY][mySelectX - 1] & RP_VALID)
            {
                --mySelectX;
                dirty = true;
            }
        }
        break;

    case MoveDirection::right:
        if (mySelectX < myWidth - 1)
        {
            if (myRoomGrid[mySelectY][mySelectX + 1] & RP_VALID)
            {
                ++mySelectX;
                dirty = true;
            }
        }
        break;
    }

    if (dirty)
    {
        renderRoom(oldX, oldY);
        renderSelectedRoom();
    }
}

void World::move()
{
    int distance = std::abs(myCurrX - mySelectX) + std::abs(myCurrY - mySelectY);

    if (distance != 1)
    {
        displayMessage(myMessages[UM_BADMOVE], 0);
        return;
    }

    int oldX = myCurrX;
    int oldY = myCurrY;

    myCurrX = mySelectX;
    myCurrY = mySelectY;

    renderRoom(oldX, oldY);
    renderRoom(myCurrX, myCurrY);

    if (myRoomGrid[myCurrY][myCurrX] & RP_WUMPUS)
    {
        displayMessage(myMessages[UM_LOSE], 0);
        displayMessage(myMessages[UM_EXIT], 1);
        myGameOver = true;
    }
    else if (myRoomGrid[myCurrY][myCurrX] & RP_TREASURE)
    {
        displayMessage(myMessages[UM_WIN], 0);
        displayMessage(myMessages[UM_EXIT], 1);
        myGameOver = true;
    }
    else if (isNearWumpus())
    {
        displayMessage(myMessages[UM_NEARWUMPUS], 0);
    }
    else
    {
        displayMessage(myMessages[UM_CLEAR], 0);
    }
}

void World::toggleWumpus()
{
    if (myRoomGrid[mySelectY][mySelectX] & RP_MARK_WUMPUS)
    {
        myRoomGrid[mySelectY][mySelectX] &= ~RP_MARK_WUMPUS;
    }
    else
    {
        myRoomGrid[mySelectY][mySelectX] &= ~RP_MARK_UNKNOWN;
        myRoomGrid[mySelectY][mySelectX] |= RP_MARK_WUMPUS;
    }

    renderSelectedRoom();
}

void World::toggleUnknown()
{
    if (myRoomGrid[mySelectY][mySelectX] & RP_MARK_UNKNOWN)
    {
        myRoomGrid[mySelectY][mySelectX] &= ~RP_MARK_UNKNOWN;
    }
    else
    {
        myRoomGrid[mySelectY][mySelectX] &= ~RP_MARK_WUMPUS;
        myRoomGrid[mySelectY][mySelectX] |= RP_MARK_UNKNOWN;
    }

    renderSelectedRoom();
}

void World::dumpRawData()
{
    myTerminal->clearScreen();

    std::ostringstream oss;
    oss << "World data:\n";

    for (int y = 0; y < myHeight; ++y)
    {
        for (int x = 0; x < myWidth; ++x)
        {
            oss << myRoomGrid[y][x] << ',';
        }

        oss.seekp(-1, oss.cur);
        oss << '\n';
    }

    myTerminal->output(oss);
}

void World::updateKernel(int x, int y) const
{
    if (x == 0)
    {
        myKernel[0][0] = myKernel[1][0] = myKernel[2][0] = false;

        if (y == 0)
        {
            myKernel[0][1] = myKernel[0][2] = false;
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[1][2] = (myRoomGrid[y][x + 1] & RP_VALID);
            myKernel[2][1] = (myRoomGrid[y + 1][x] & RP_VALID);
            myKernel[2][2] = (myRoomGrid[y + 1][x + 1] & RP_VALID);
        }
        else if (y < myHeight - 1)
        {
            myKernel[0][1] = (myRoomGrid[y - 1][x] & RP_VALID);
            myKernel[0][2] = (myRoomGrid[y - 1][x + 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[1][2] = (myRoomGrid[y][x + 1] & RP_VALID);
            myKernel[2][1] = (myRoomGrid[y + 1][x] & RP_VALID);
            myKernel[2][2] = (myRoomGrid[y + 1][x + 1] & RP_VALID);
        }
        else
        {
            myKernel[0][1] = (myRoomGrid[y - 1][x] & RP_VALID);
            myKernel[0][2] = (myRoomGrid[y - 1][x + 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[1][2] = (myRoomGrid[y][x + 1] & RP_VALID);
            myKernel[2][1] = myKernel[2][2] = false;
        }
    }
    else if (x < myWidth - 1) 
    {
        if (y == 0)
        {
            myKernel[0][0] = myKernel[0][1] = myKernel[0][2] = false;
            myKernel[1][0] = (myRoomGrid[y][x - 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[1][2] = (myRoomGrid[y][x + 1] & RP_VALID);
            myKernel[2][0] = (myRoomGrid[y + 1][x - 1] & RP_VALID);
            myKernel[2][1] = (myRoomGrid[y + 1][x] & RP_VALID);
            myKernel[2][2] = (myRoomGrid[y + 1][x + 1] & RP_VALID);
        }
        else if (y < myHeight - 1)
        {
            myKernel[0][0] = (myRoomGrid[y - 1][x - 1] & RP_VALID);
            myKernel[0][1] = (myRoomGrid[y - 1][x] & RP_VALID);
            myKernel[0][2] = (myRoomGrid[y - 1][x + 1] & RP_VALID);
            myKernel[1][0] = (myRoomGrid[y][x - 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[1][2] = (myRoomGrid[y][x + 1] & RP_VALID);
            myKernel[2][0] = (myRoomGrid[y + 1][x - 1] & RP_VALID);
            myKernel[2][1] = (myRoomGrid[y + 1][x] & RP_VALID);
            myKernel[2][2] = (myRoomGrid[y + 1][x + 1] & RP_VALID);
        }
        else
        {
            myKernel[0][0] = (myRoomGrid[y - 1][x - 1] & RP_VALID);
            myKernel[0][1] = (myRoomGrid[y - 1][x] & RP_VALID);
            myKernel[0][2] = (myRoomGrid[y - 1][x + 1] & RP_VALID);
            myKernel[1][0] = (myRoomGrid[y][x - 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[1][2] = (myRoomGrid[y][x + 1] & RP_VALID);
            myKernel[2][0] = myKernel[2][1] = myKernel[2][2] = false;
        }
    }
    else
    {
        myKernel[0][2] = myKernel[1][2] = myKernel[2][2] = false;

        if (y == 0)
        {
            myKernel[0][0] = myKernel[0][1] = false;
            myKernel[1][0] = (myRoomGrid[y][x - 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[2][0] = (myRoomGrid[y + 1][x - 1] & RP_VALID);
            myKernel[2][1] = (myRoomGrid[y + 1][x] & RP_VALID);
        }
        else if (y < myHeight - 1)
        {
            myKernel[0][0] = (myRoomGrid[y - 1][x - 1] & RP_VALID);
            myKernel[0][1] = (myRoomGrid[y - 1][x] & RP_VALID);
            myKernel[1][0] = (myRoomGrid[y][x - 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[2][0] = (myRoomGrid[y + 1][x - 1] & RP_VALID);
            myKernel[2][1] = (myRoomGrid[y + 1][x] & RP_VALID);
        }
        else
        {
            myKernel[0][0] = (myRoomGrid[y - 1][x - 1] & RP_VALID);
            myKernel[0][1] = (myRoomGrid[y - 1][x] & RP_VALID);
            myKernel[1][0] = (myRoomGrid[y][x - 1] & RP_VALID);
            myKernel[1][1] = (myRoomGrid[y][x] & RP_VALID);
            myKernel[2][0] = myKernel[2][1] = false;
        }
    }
}

std::string World::getCornerStyle(int xKernel, int yKernel, int drawStyle) const
{
    int cornerIndex = 0;

    if (myKernel[yKernel][xKernel])
    {
        cornerIndex |= RA_TOPLEFT;
    }

    if (myKernel[yKernel][xKernel + 1])
    {
        cornerIndex |= RA_TOPRIGHT;
    }

    if (myKernel[yKernel + 1][xKernel])
    {
        cornerIndex |= RA_BOTTOMLEFT;
    }

    if (myKernel[yKernel + 1][xKernel + 1])
    {
        cornerIndex |= RA_BOTTOMRIGHT;
    }

    return myCornerStyles[cornerIndex][drawStyle];
}

std::string World::getRoomContent(int x, int y) const
{
    if ((x == myCurrX) && (y == myCurrY))
    {
        return mySpecialSymbols[SS_FACE];
    }
    else if (myRoomGrid[y][x] & RP_LOCKED)
    {
        return mySpecialSymbols[SS_LOCKED];
    }
    else if (myRoomGrid[y][x] & RP_MARK_WUMPUS)
    {
        return mySpecialSymbols[SS_WUMPUS];
    }
    else if (myRoomGrid[y][x] & RP_MARK_UNKNOWN)
    {
        return mySpecialSymbols[SS_UNKNOWN];
    }
    else
    {
        return " ";
    }
}

bool World::isNearWumpus() const
{
    if ((myCurrX > 0) && (myRoomGrid[myCurrY][myCurrX - 1] & RP_WUMPUS))
    {
        return true;
    }

    if ((myCurrX < myWidth - 1) && (myRoomGrid[myCurrY][myCurrX + 1] & RP_WUMPUS))
    {
        return true;
    }

    if ((myCurrY > 0) && (myRoomGrid[myCurrY - 1][myCurrX] & RP_WUMPUS))
    {
        return true;
    }

    if ((myCurrY < myHeight - 1) && (myRoomGrid[myCurrY + 1][myCurrX] & RP_WUMPUS))
    {
        return true;
    }

    return false;
}

void World::displayMessage(const std::string & message, int messageLine) const
{
    myTerminal->setCursorPos(0, (myHeight * 2) + 1 + messageLine);
    myTerminal->output(message);
}
