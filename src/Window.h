#ifndef Window_hpp
#define Window_hpp
#include <SFML/Graphics.hpp>
#include "Map.h"
#include "Player.h"
#include "Engine.h"

// colors
const sf::Color transparent_white(255, 255, 255, 125);

void drawLines(sf::RenderWindow &window, sf::RenderStates);
void drawMinimap(sf::RenderWindow &window);

#endif