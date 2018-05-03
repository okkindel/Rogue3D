#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include "Player.h"

const int screenWidth = 1280;
const int screenHeight = 768;
// height of player camera (1.0 is ceiling, 0.0 is floor)
const float cameraHeight = 0.5f;
// size of texture plane
const int texture_size = 512;
// size of each wall type in the full texture
const int texture_wall_size = 128;
// time between FPS text refresh. FPS is smoothed out over this time
const float fps_refresh_time = 0.05;
// minimap scale
const int map_scale = 8;

// colors
const sf::Color transparent_white(255, 255, 255, 125);
const sf::Color color_brick(85, 55, 50);

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

int renderer()
{

    // if the map is not correct, we can have segmentation faults. So check it.
    if (!checkMap())
    {
        fprintf(stderr, "Map is invalid!\n");
        return EXIT_FAILURE;
    }

    sf::Font font;
    if (!font.loadFromFile("data/font/opensans.ttf"))
    {
        fprintf(stderr, "Cannot open font!\n");
        return EXIT_FAILURE;
    }

    sf::Texture texture;
    if (!texture.loadFromFile("data/texture/walls.png"))
    {
        fprintf(stderr, "Cannot open texture!\n");
        return EXIT_FAILURE;
    }

    // render state that uses the texture
    sf::RenderStates state(&texture);

    // create window
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Roguelike");
    window.setSize(sf::Vector2u(screenWidth, screenHeight));

    window.setFramerateLimit(60);
    bool hasFocus = true;

    // lines used to draw walls on the screen
    sf::VertexArray lines(sf::Lines);
    // lines of minimap
    sf::VertexArray maplines(sf::Lines);
    // lines of cellings and flores
    sf::VertexArray floorlines(sf::Lines);

    sf::Text fpsText("", font, 50); // text object for FPS counter
    fpsText.setPosition(screenWidth - 250, 10);
    sf::Clock clock;                              // timer
    char frameInfoString[sizeof("FPS: *****.*")]; // string buffer for frame information

    float dt_counter = 0.0f;      // delta time for multiple frames, for calculating FPS smoothly
    int frame_counter = 0;        // counts frames for FPS calculation
    int64_t frame_time_micro = 0; // time needed to draw frames in microseconds

    while (window.isOpen())
    {
        // get delta time
        float dt = clock.restart().asSeconds();

        // Update FPS, smoothed over time
        if (dt_counter >= fps_refresh_time)
        {
            float fps = (float)frame_counter / dt_counter;
            frame_time_micro /= frame_counter;
            snprintf(frameInfoString, sizeof(frameInfoString), "FPS: %3.1f", fps);
            fpsText.setString(frameInfoString);
            dt_counter = 0.0f;
            frame_counter = 0;
            frame_time_micro = 0;
        }
        dt_counter += dt;
        ++frame_counter;

        // handle SFML events
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::LostFocus:
                hasFocus = false;
                break;
            case sf::Event::GainedFocus:
                hasFocus = true;
                break;
            default:
                break;
            }
        }

        // handle keyboard input
        if (hasFocus)
        {
            handleMove(dt);
        }

        lines.resize(0);

        // loop through vertical screen lines, draw a line of wall for each
        for (int x = 0; x < screenWidth; ++x)
        {

            // ray to emit
            float cameraX = 2 * x / (float)screenWidth - 1.0f; // x in camera space (between -1 and +1)
            sf::Vector2f rayPos = getPosition();
            sf::Vector2f rayDir = getDirection() + getPlane() * cameraX;

            // NOTE: with floats, division by zero gives you the "infinity" value. This code depends on this.

            // calculate distance traversed between each grid line for x and y based on direction
            sf::Vector2f deltaDist(
                sqrt(1.0f + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)),
                sqrt(1.0f + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y)));

            // which box of the map we're in
            sf::Vector2i mapPos(rayPos);
            // what direction to step in (+1 or -1 for each dimension)
            sf::Vector2i step;
            // distance from current position to next gridline, for x and y separately
            sf::Vector2f sideDist;

            // calculate step and initial sideDist
            if (rayDir.x < 0.0f)
            {
                step.x = -1;
                sideDist.x = (rayPos.x - mapPos.x) * deltaDist.x;
            }
            else
            {
                step.x = 1;
                sideDist.x = (mapPos.x + 1.0f - rayPos.x) * deltaDist.x;
            }
            if (rayDir.y < 0.0f)
            {
                step.y = -1;
                sideDist.y = (rayPos.y - mapPos.y) * deltaDist.y;
            }
            else
            {
                step.y = 1;
                sideDist.y = (mapPos.y + 1.0f - rayPos.y) * deltaDist.y;
            }

            // tile type that got hit
            char tile = '.';
            // did we hit a horizontal side? Otherwise it's vertical
            bool horizontal;
            // wall distance, projected on camera direction
            float distance = 0.0f;
            // height of wall to draw on the screen at each distance
            int wallHeight;
            // position of ceiling pixel on the screen
            int ceilingPixel = 0;
            // position of ground pixel on the screen
            int groundPixel = screenHeight;

            // cast the ray until we hit a wall, meanwhile draw floors
            while (tile == '.')
            {
                if (sideDist.x < sideDist.y)
                {
                    sideDist.x += deltaDist.x;
                    mapPos.x += step.x;
                    horizontal = true;
                    distance = (mapPos.x - rayPos.x + (1 - step.x) / 2) / rayDir.x;
                }
                else
                {
                    sideDist.y += deltaDist.y;
                    mapPos.y += step.y;
                    horizontal = false;
                    distance = (mapPos.y - rayPos.y + (1 - step.y) / 2) / rayDir.y;
                }

                wallHeight = screenHeight / distance;

                sf::Color cell_color = sf::Color::White;
                cell_color.r /= distance;
                cell_color.g /= distance;
                cell_color.b /= distance;

                sf::Color floor_color = color_brick;
                floor_color.r /= distance;
                floor_color.g /= distance;
                floor_color.b /= distance;

                // add floor
                floorlines.append(sf::Vertex(sf::Vector2f((float)x, (float)groundPixel), floor_color));
                groundPixel = int(wallHeight * cameraHeight + screenHeight * 0.5f);
                floorlines.append(sf::Vertex(sf::Vector2f((float)x, (float)groundPixel), floor_color));

                // add ceiling
                floorlines.append(sf::Vertex(sf::Vector2f((float)x, (float)ceilingPixel), cell_color));
                ceilingPixel = int(-wallHeight * (1.0f - cameraHeight) + screenHeight * 0.5f);
                floorlines.append(sf::Vertex(sf::Vector2f((float)x, (float)ceilingPixel), cell_color));

                tile = getTile(mapPos.x, mapPos.y);
            }

            // add rays to map lines
            maplines.append(sf::Vertex(sf::Vector2f(10 + (map_scale - 3) / 2 + rayPos.x * (map_scale - 0.1),
                                                    10 + (map_scale - 3) / 2 + rayPos.y * (map_scale - 0.1)),
                                       sf::Color::Green));
            maplines.append(sf::Vertex(sf::Vector2f(10 + (map_scale - 3) / 2 + (rayPos.x + distance * rayDir.x) * (map_scale - 0.1),
                                                    10 + (map_scale - 3) / 2 + (rayPos.y + distance * rayDir.y) * (map_scale - 0.1)),
                                       sf::Color::Black));

            // calculate lowest and highest pixel to fill in current line
            int drawStart = ceilingPixel;
            int drawEnd = groundPixel;

            // get position of the wall texture in the full texture
            int wallTextureNum = (int)wallTypes.find(tile)->second;
            sf::Vector2i texture_coords(
                wallTextureNum * texture_wall_size % texture_size,
                wallTextureNum * texture_wall_size / texture_size * texture_wall_size);

            // calculate where the wall was hit
            float wall_x;
            if (horizontal)
            {
                wall_x = rayPos.y + distance * rayDir.y;
            }
            else
            {
                wall_x = rayPos.x + distance * rayDir.x;
            }
            wall_x -= floor(wall_x);

            // get x coordinate on the wall texture
            int tex_x = int(wall_x * float(texture_wall_size));

            // flip texture if we see it on the other side of us, this prevents a mirrored effect for the texture
            if ((horizontal && rayDir.x <= 0) || (!horizontal && rayDir.y >= 0))
            {
                tex_x = texture_wall_size - tex_x - 1;
            }

            texture_coords.x += tex_x;

            // illusion of shadows by making horizontal walls darker
            sf::Color color = sf::Color::White;
            if (horizontal)
            {
                color.r /= 1.5;
                color.g /= 1.5;
                color.b /= 1.5;
            }

            // dynamic shadows
            if (distance > 1)
            {
                color.r /= (distance);
                color.g /= (distance);
                color.b /= (distance);
            }

            // add line to vertex buffer
            lines.append(sf::Vertex(
                sf::Vector2f((float)x, (float)drawStart),
                color,
                sf::Vector2f((float)texture_coords.x, (float)texture_coords.y + 1)));
            lines.append(sf::Vertex(
                sf::Vector2f((float)x, (float)drawEnd),
                color,
                sf::Vector2f((float)texture_coords.x, (float)(texture_coords.y + texture_wall_size - 1))));
        }

        // draw everything
        window.clear();
        window.draw(lines, state);
        window.draw(floorlines);
        window.draw(maplines);
        window.draw(fpsText);
        floorlines.clear();
        maplines.clear();

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
        rectangle.setFillColor(sf::Color::White);
        // not very accurate values but less of math
        rectangle.setSize(sf::Vector2f((map_scale - 3), (map_scale - 3)));
        sf::Vector2f pos = getPosition();
        rectangle.setPosition(10 + pos.x * (map_scale - 0.1), 10 + pos.y * (map_scale - 0.1));
        window.draw(rectangle);

        frame_time_micro += clock.getElapsedTime().asMicroseconds();
        window.display();
    }

    return EXIT_SUCCESS;
}

int main()
{
    renderer();
}
