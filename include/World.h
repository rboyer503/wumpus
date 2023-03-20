#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <memory>
#include <string>

class ITerminal;


enum eRoomProp
{
    RP_VALID = 1 << 0,
    RP_WUMPUS = 1 << 1,
    RP_KEY = 1 << 2,
    RP_LOCKED = 1 << 3,
    RP_TREASURE = 1 << 4,
    RP_DOOR = 1 << 5,
    RP_MARK_WUMPUS = 1 << 6,
    RP_MARK_UNKNOWN = 1 << 7
};

enum eRoomAdjacency
{
    RA_TOPLEFT = 1 << 0,
    RA_TOPRIGHT = 1 << 1,
    RA_BOTTOMLEFT = 1 << 2,
    RA_BOTTOMRIGHT = 1 << 3
};

enum eLineStyle
{
    LS_HORIZONTAL,
    LS_LEFT_VERT,
    LS_RIGHT_VERT
};

enum eDrawStyle
{
    DS_SINGLE,
    DS_DOUBLE
};

enum eSpecialSymbol
{
    SS_FACE,
    SS_WUMPUS,
    SS_KEY,
    SS_LOCKED,
    SS_UNKNOWN
};

enum eUserMessage
{
    UM_CLEAR,
    UM_BADMOVE,
    UM_NEARWUMPUS,
    UM_LOSE,
    UM_WIN,
    UM_EXIT
};

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
    static const std::string myMessages[];

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
