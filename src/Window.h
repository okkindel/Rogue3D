#ifndef Window_hpp
#define Window_hpp
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Map.h"
#include "Engine.h"

// colors
const sf::Color transparent_white(255, 255, 255, 125);

void handleKeys();
void drawLines(sf::RenderWindow &window, sf::RenderStates);
void drawMinimap(sf::RenderWindow &window);

#endif