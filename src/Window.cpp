#include "Window.h"

void drawLines(sf::RenderWindow &window, sf::RenderStates state)
{
    // draw walls, state - textures
    window.draw(getLines(), state);
    // draw ceiling and flooor
    window.draw(getFloorLines());
    // draw player on minimap
    window.draw(getMapLines());
}

void drawMinimap(sf::RenderWindow &window)
{
    // draw minimap
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(map_scale, map_scale));
    for (int i = 0; i < mapHeight; i++)
    {
        for (int j = 0; j < mapWidth; j++)
        {
            rectangle.setPosition(10 + j * map_scale, 10 + i * map_scale);
            if (getTile(j, i) != '.')
            {
                rectangle.setFillColor(sf::Color::Black);
                window.draw(rectangle);
            }
            else
            {
                rectangle.setFillColor(transparent_white);
                window.draw(rectangle);
            }
        }
    }

    //draw player
    rectangle.setFillColor(sf::Color::Magenta);
    // not very accurate values but less of math
    rectangle.setSize(sf::Vector2f((map_scale - 3), (map_scale - 3)));
    sf::Vector2f pos = getPosition();
    rectangle.setPosition(10 + pos.x * (map_scale - 0.1), 10 + pos.y * (map_scale - 0.1));
    window.draw(rectangle);
}