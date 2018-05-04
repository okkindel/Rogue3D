#include "Player.h"

sf::Vector2f position(15.5f, 16.5f); // coordinates in worldMap
sf::Vector2f direction(1.0f, 0.0f);  // direction, relative to (0,0)
sf::Vector2f plane(0.0f, 0.66f);    // 2d raycaster version of the camera plane,

// check if a rectangular thing with given size can move to given position without colliding with walls or
// being outside of the map
// position is considered the middle of the rectangle
bool canMove(sf::Vector2f position) {
    // player collision box width and height, derived from collision_box must be perpendicular to rotation
    sf::Vector2f size(collision_box, collision_box);
    // create the corners of the rectangle
    sf::Vector2i upper_left(position - size / 2.0f);
    sf::Vector2i lower_right(position + size / 2.0f);
    if (upper_left.x < 0 || upper_left.y < 0 || lower_right.x >= mapWidth || lower_right.y >= mapHeight) {
        return false; // out of map bounds
    }
    // loop through each map tile within the rectangle. The rectangle could be multiple tiles in size!
    for (int y = upper_left.y; y <= lower_right.y; ++y) {
        for (int x = upper_left.x; x <= lower_right.x; ++x) {
            if (getTile(x, y) != '.') {
                return false;
            }
        }
    }
    return true;
}

// rotate a given vector with given float value in radians and return the result
sf::Vector2f rotateVec(sf::Vector2f vec, float value) {
    return sf::Vector2f(
            vec.x * std::cos(value) - vec.y * std::sin(value),
            vec.x * std::sin(value) + vec.y * std::cos(value));
}

void handleMove(float dt) {
    using kb = sf::Keyboard;

    // moving forward or backwards (1.0 or -1.0)
    float moveDirection = 0.0f;
    // rotating rightwards or leftwards(1.0 or -1.0)
    float rotateDirection = 0.0f;
    // vertical move or shifted
    bool vertical = true;

    if (kb::isKeyPressed(kb::Left))
        rotateDirection = -1.0f;
    else if (kb::isKeyPressed(kb::Right))
        rotateDirection = 1.0f;
    if (kb::isKeyPressed(kb::LShift))
        vertical = false;
    if (kb::isKeyPressed(kb::Up))
        moveDirection = 1.0f;
    else if (kb::isKeyPressed(kb::Down))
        moveDirection = -1.0f;

    // handle movement
    if (moveDirection != 0.0f && vertical) {
        sf::Vector2f moveVec = direction * moveSpeed * moveDirection * dt;

        if (canMove(sf::Vector2f(position.x + moveVec.x, position.y)))
            position.x += moveVec.x;
        if (canMove(sf::Vector2f(position.x, position.y + moveVec.y)))
            position.y += moveVec.y;
    }

    // handle rotation
    if (rotateDirection != 0.0f) {
        float rotation = rotateSpeed * rotateDirection * dt;
        if (vertical) {
            direction = rotateVec(direction, rotation);
            plane = rotateVec(plane, rotation);
        } else {
            sf::Vector2f dir = rotateVec(direction, rotation + M_PI / 2);
            sf::Vector2f moveVec = dir * moveSpeed * rotateDirection * dt;

            if (canMove(sf::Vector2f(position.x + moveVec.x, position.y)))
                position.x += moveVec.x;
            if (canMove(sf::Vector2f(position.x, position.y + moveVec.y)))
                position.y += moveVec.y;
        }
    }
}

sf::Vector2f getPosition() {
    return position;
}

sf::Vector2f getDirection() {
    return direction;
}

sf::Vector2f getPlane() {
    return plane;
}