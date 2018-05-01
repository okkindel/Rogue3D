#include <unordered_map>
#include "tiles.h"

// list of wall texture types, in order as they appear in the full texture
enum class WallTexture
{
    Wall,
    Bush,
    Door
};

// valid wall types and their texture for the world map
const std::unordered_map<char, WallTexture> wallTypes{
    {'1', WallTexture::Wall},
    {'2', WallTexture::Bush},
    {'3', WallTexture::Door},
};

// size of the top-down world map in tiles
const int mapWidth = 32;
const int mapHeight = 32;

const char worldMap[] =
    "11111111111111111111111111111111"
    "1.1..1..3..3..1..11.1.11..11.1.1"
    "1.1.11.11..11.1..1..1..1..1..1.1"
    "1111.......1..1.111...111.1..1.1"
    "1.1.11..11.1..11.1....11.1111111"
    "1111111.1.111.1.....11....1..1.1"
    "1111....11.1..1..1..1..1..1..1.1"
    "1.1..1.....1..1.111111...121.1.1"
    "1.1.....11......11111.113.11...1"
    "1.1..1..1..11.1..1.111.1.....111"
    "1.1..1.121....1.111112111......1"
    "1112...11..11.1..1....11..11.1.1"
    "1....1.....1.111.......1..1..111"
    "1.1..1.11.111...11..11...11.1111"
    "1.1..1..1........1..1.......1111"
    "1.1..11.2..1..1..11....2.....111"
    "1....1.111.1..1.....1..1.111.1.1"
    "1.1..11.1.111.1..11.1..1..1..1.1"
    "1.1.....1.111.1.111.1.111....2.1"
    "1.1.11.111111....11...111...1111"
    "1.1..1.111111.1..1..1..1..1....1"
    "1.3..1.111.1..1.111.1..1....2111"
    "1111...111.31....1.111.1.111.111"
    "1.1.....1.......11.111.1..1.1111"
    "1.1..1.111...111.1.1111131111111"
    "1111111....11....1....111.1....1"
    "1.3..1.....11.1.....1..1..1..1.1"
    "1....11...111111......111.1..1.1"
    "1.1.111...121....11....1..11.1.1"
    "1....11.1..1..1........1.......1"
    "1111111.1....111...22.111....1.1"
    "11111111111111111111111111111111";

// get a tile from worldMap. Not memory safe.
char getTile(int x, int y)
{
    return worldMap[y * mapWidth + x];
}

// checks worldMap for errors
// returns: true on success, false on errors found
bool checkMap()
{
    // check size
    int mapSize = sizeof(worldMap) - 1; // - 1 because sizeof also counts the final NULL character
    if (mapSize != mapWidth * mapHeight)
    {
        fprintf(stderr, "Map size(%d) is not mapWidth * mapHeight(%d)\n", mapSize, mapWidth * mapHeight);
        return false;
    }

    for (int y = 0; y < mapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            char tile = getTile(x, y);
            // check if tile type is valid
            if (tile != '.' && wallTypes.find(tile) == wallTypes.end())
            {
                fprintf(stderr, "map tile at [%3d,%3d] has an unknown tile type(%c)\n", x, y, tile);
                return false;
            }
            // check if edges are walls
            if ((y == 0 || x == 0 || y == mapHeight - 1 || x == mapWidth - 1) &&
                tile == '.')
            {
                fprintf(stderr, "map edge at [%3d,%3d] is a floor (should be wall)\n", x, y);
                return false;
            }
        }
    }
    return true;
}