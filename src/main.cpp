#include <stdio.h>
#include <stdint.h>
#include <SFML/Graphics.hpp>
#include "Engine.h"

// time between FPS text refresh. FPS is smoothed out over this time
const float fps_refresh_time = 0.05;

// colors
const sf::Color transparent_white(255, 255, 255, 125);

int draw() {

    // if the map is not correct, we can have segmentation faults. So check it.
    if (!checkMap()) {
        fprintf(stderr, "Map is invalid!\n");
        return EXIT_FAILURE;
    }

    sf::Font font;
    if (!font.loadFromFile("data/font/opensans.ttf")) {
        fprintf(stderr, "Cannot open font!\n");
        return EXIT_FAILURE;
    }

    sf::Texture texture;
    if (!texture.loadFromFile("data/texture/walls.png")) {
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

    sf::Text fpsText("", font, 50); // text object for FPS counter
    fpsText.setPosition(screenWidth - 250, 10);
    sf::Clock clock;                              // timer
    char frameInfoString[sizeof("FPS: *****.*")]; // string buffer for frame information

    float dt_counter = 0.0f;      // delta time for multiple frames, for calculating FPS smoothly
    int frame_counter = 0;        // counts frames for FPS calculation
    int64_t frame_time_micro = 0; // time needed to draw frames in microseconds

    while (window.isOpen()) {
        // get delta time
        float dt = clock.restart().asSeconds();

        // Update FPS, smoothed over time
        if (dt_counter >= fps_refresh_time) {
            float fps = (float) frame_counter / dt_counter;
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
        while (window.pollEvent(event)) {
            switch (event.type) {
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
        if (hasFocus) {
            handleMove(dt);
        }

        getLines().resize(0);

        render();

        // draw everything
        window.clear();
        window.draw(getLines(), state);
        window.draw(getFloorLines());
        window.draw(getMapLines());
        window.draw(fpsText);
        clearAllLines();

        // draw minimap
        sf::RectangleShape rectangle;
        rectangle.setSize(sf::Vector2f(map_scale, map_scale));
        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++) {
                rectangle.setPosition(10 + j * map_scale, 10 + i * map_scale);
                if (getTile(j, i) != '.') {
                    rectangle.setFillColor(sf::Color::Black);
                    window.draw(rectangle);
                } else {
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

int main() {
    draw();
}
