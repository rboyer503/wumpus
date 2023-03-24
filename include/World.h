#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <memory>
#include <string>

class ITerminal;


namespace RoomProp
{
    enum
    {
        VALID = 1 << 0,
        WUMPUS = 1 << 1,
        KEY = 1 << 2,
        LOCKED = 1 << 3,
        TREASURE = 1 << 4,
        DOOR = 1 << 5,
        MARK_WUMPUS = 1 << 6,
        MARK_UNKNOWN = 1 << 7
    };
}

namespace RoomAdjacency
{
    enum
    {
        TOPLEFT = 1 << 0,
        TOPRIGHT = 1 << 1,
        BOTTOMLEFT = 1 << 2,
        BOTTOMRIGHT = 1 << 3
    };
}

namespace LineStyle
{
    enum
    {
        HORIZONTAL,
        LEFT_VERT,
        RIGHT_VERT
    };
}

namespace DrawStyle
{
    enum
    {
        SINGLE,
        DOUBLE
    };
}

namespace SpecialSymbol
{
    enum
    {
        FACE,
        WUMPUS,
        KEY,
        LOCKED,
        UNKNOWN
    };
}

namespace WorldMessage
{
    enum
    {
        CLEAR,
        BADMOVE,
        NEARWUMPUS,
        LOSE,
        WIN,
        EXIT,
        MAX
    };
}

using room_data_t = uint16_t;


// Class managing the "world", a set of rooms with various properties.
class World
{
public:
    struct RawData
    {
        int width;
        int height;
        int startX;
        int startY;
        const room_data_t * data;
    };

    enum class MoveDirection
    {
        up,
        down,
        left,
        right
    };

    World(ITerminal * terminal);

    void load(const RawData & rawData);
    void render() const;
    void renderRoom(int x, int y) const;
    void renderSelectedRoom() const;

    void moveSelection(MoveDirection direction);
    void move();
    void toggleWumpus();
    void toggleUnknown();

    bool isGameOver() const
    {
        return myGameOver;
    }

    void dumpRawData();

private:
    void updateKernel(int x, int y) const;
    std::string getCornerStyle(int xKernel, int yKernel, int drawStyle) const;
    std::string getRoomContent(int x, int y) const;
    bool isNearWumpus() const;
    void displayMessage(const std::string & message, int messageLine) const;

    static const room_data_t myDefaultRoomData[];
    static const RawData myDefaultRawData;
    static const std::string myCornerStyles[][2];
    static const std::string myLineStyles[][2];
    static const std::string mySpecialSymbols[];
    static const std::string myMessages[WorldMessage::MAX];

    ITerminal * myTerminal = nullptr;
    int myWidth = 0;
    int myHeight = 0;
    int myCurrX = 0;
    int myCurrY = 0;
    int mySelectX = 0;
    int mySelectY = 0;
    bool myGameOver = false;
    std::unique_ptr<room_data_t[]> myRoomData;
    std::unique_ptr<room_data_t*[]> myRoomGrid;
    mutable bool myKernel[3][3];
};

#endif // WORLD_H
