#ifndef Player_hpp
#define Player_hpp
#include <SFML/Graphics.hpp>
#include "Map.h"

const float collision_box = 0.375; // dimensions of player collision box, in tiles
const float moveSpeed = 3.5;       // player movement speed in tiles per second
const float rotateSpeed = 2.0;     // player rotation speed in radians per second

bool canMove(sf::Vector2f);
void handleMove(float);
sf::Vector2f rotateVec(sf::Vector2f, float);
sf::Vector2f getPosition();
sf::Vector2f getDirection();
sf::Vector2f getPlane();

#endif