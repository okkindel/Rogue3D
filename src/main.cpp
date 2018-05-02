#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "map.cpp"
#include "player.cpp"
#include "tiles.h"

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

    // player
    sf::Vector2f position(15.5f, 16.5f); // coordinates in worldMap
    sf::Vector2f direction(0.0f, 1.0f);  // direction, relative to (0,0)
    sf::Vector2f plane(-0.66f, 0.0f);    // 2d raycaster version of the camera plane,
                                         // must be perpendicular to rotation
    float size_f = 0.375f;               // dimensions of player collision box, in tiles
    float moveSpeed = 3.5f;              // player movement speed in tiles per second
    float rotateSpeed = 3.0f;            // player rotation speed in radians per second

    sf::Vector2f size(size_f, size_f); // player collision box width and height, derived from size_f

    // create window
    sf::RenderWindow window(sf::VideoMode(screenWidth + 1, screenHeight), "Roguelike");
    window.setSize(sf::Vector2u(screenWidth, screenHeight)); // why add +1 and then set the size correctly?
                                                             // Fixes some problem with the viewport. If you
                                                             // don't do it, you'll see lots of gaps. Maybe
                                                             // there's a better fix.

    window.setFramerateLimit(60);
    bool hasFocus = true;

    // lines used to draw walls and floors on the screen
    sf::VertexArray lines(sf::Lines, 18 * screenWidth);

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
            using kb = sf::Keyboard;

            // moving forward or backwards (1.0 or -1.0)
            float moveForward = 0.0f;

            // get input
            if (kb::isKeyPressed(kb::Up))
            {
                moveForward = 1.0f;
            }
            else if (kb::isKeyPressed(kb::Down))
            {
                moveForward = -1.0f;
            }

            // handle movement
            if (moveForward != 0.0f)
            {
                sf::Vector2f moveVec = direction * moveSpeed * moveForward * dt;

                if (canMove(sf::Vector2f(position.x + moveVec.x, position.y), size))
                {
                    position.x += moveVec.x;
                }
                if (canMove(sf::Vector2f(position.x, position.y + moveVec.y), size))
                {
                    position.y += moveVec.y;
                }
            }

            // rotating rightwards or leftwards(1.0 or -1.0)
            float rotateDirection = 0.0f;

            // get input
            if (kb::isKeyPressed(kb::Left))
            {
                rotateDirection = -1.0f;
            }
            else if (kb::isKeyPressed(kb::Right))
            {
                rotateDirection = 1.0f;
            }

            // handle rotation
            if (rotateDirection != 0.0f)
            {
                float rotation = rotateSpeed * rotateDirection * dt;
                direction = rotateVec(direction, rotation);
                plane = rotateVec(plane, rotation);
            }
        }

        lines.resize(0);

        // loop through vertical screen lines, draw a line of wall for each
        for (int x = 0; x < screenWidth; ++x)
        {

            // ray to emit
            float cameraX = 2 * x / (float)screenWidth - 1.0f; // x in camera space (between -1 and +1)
            sf::Vector2f rayPos = position;
            sf::Vector2f rayDir = direction + plane * cameraX;

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

                sf::Color cell_color = sf::Color::Green;
                cell_color.r /= distance;
                cell_color.g /= distance;
                cell_color.b /= distance;

                // add floor
                lines.append(sf::Vertex(sf::Vector2f((float)x, (float)groundPixel), cell_color, sf::Vector2f(60.0f, 60.0f)));
                groundPixel = int(wallHeight * cameraHeight + screenHeight * 0.5f);
                lines.append(sf::Vertex(sf::Vector2f((float)x, (float)groundPixel), cell_color, sf::Vector2f(60.0f, 60.0f)));

                // add ceiling
                lines.append(sf::Vertex(sf::Vector2f((float)x, (float)ceilingPixel), cell_color, sf::Vector2f(385.0f, 129.0f)));
                ceilingPixel = int(-wallHeight * (1.0f - cameraHeight) + screenHeight * 0.5f);
                lines.append(sf::Vertex(sf::Vector2f((float)x, (float)ceilingPixel), cell_color, sf::Vector2f(385.0f, 129.0f)));

                tile = getTile(mapPos.x, mapPos.y);
            }

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
        window.clear();
        window.draw(lines, state);

        // draw minimap
        sf::RectangleShape rectangle;
        rectangle.setSize(sf::Vector2f(8, 8));
        rectangle.setFillColor(sf::Color::Black);
        for (int i = 0; i < mapHeight; i++)
        {
            for (int j = 0; j < mapWidth; j++)
            {
                rectangle.setPosition(10 + j * 8, 10 + i * 8);
                if (getTile(j, i) != '.')
                    window.draw(rectangle);
            }
        }
        rectangle.setFillColor(sf::Color::White);
        rectangle.setSize(sf::Vector2f(5, 5));
        rectangle.setPosition(10 + position.x * 7.75, 10 + position.y * 7.75);
        window.draw(rectangle);

        window.draw(fpsText);
        frame_time_micro += clock.getElapsedTime().asMicroseconds();
        window.display();
    }

    return EXIT_SUCCESS;
}

int main()
{
    renderer();
}
