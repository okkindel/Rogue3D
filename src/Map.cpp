#include "Map.h"

// get a tile from worldMap. Not memory safe.
char getTile(int x, int y) {
    return worldMap[y * mapWidth + x];
}

// checks worldMap for errors
// returns: true on success, false on errors found
bool checkMap() {
    // check size
    int mapSize = sizeof(worldMap) - 1; // - 1 because sizeof also counts the final NULL character
    if (mapSize != mapWidth * mapHeight) {
        fprintf(stderr, "Map size(%d) is not mapWidth * mapHeight(%d)\n", mapSize, mapWidth * mapHeight);
        return false;
    }

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            char tile = getTile(x, y);
            // check if tile type is valid
            if (tile != '.' && wallTypes.find(tile) == wallTypes.end()) {
                fprintf(stderr, "map tile at [%3d,%3d] has an unknown tile type(%c)\n", x, y, tile);
                return false;
            }
            // check if edges are walls
            if ((y == 0 || x == 0 || y == mapHeight - 1 || x == mapWidth - 1) &&
                tile == '.') {
                fprintf(stderr, "map edge at [%3d,%3d] is a floor (should be wall)\n", x, y);
                return false;
            }
        }
    }
    return true;
}