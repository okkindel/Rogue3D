#ifndef Engine_hpp
#define Engine_hpp
#include <SFML/Graphics.hpp>
#include "Player.h"

// screen width
const int screenWidth = 1280;
// screen height
const int screenHeight = 768;
// height of player camera (1.0 is ceiling, 0.0 is floor)
const float cameraHeight = 0.5f;
// size of texture plane
const int texture_size = 512;
// size of each wall type in the full texture
const int texture_wall_size = 128;
// minimap scale
const int map_scale = 8;

// colors
const sf::Color color_brick(85, 55, 50);

void render();
sf::VertexArray getLines();
sf::VertexArray getMapLines();
sf::VertexArray getFloorLines();
void clearAllLines();

#endif