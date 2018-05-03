#include "Map.h"

// get a tile from worldMap. Not memory safe.
char getTile(int x, int y)
{
    return worldMap[y * mapWidth + x];
}