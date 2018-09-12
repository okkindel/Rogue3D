#include "Engine.h"

// lines used to draw walls on the screen
sf::VertexArray lines(sf::Lines);
// lines of minimap
sf::VertexArray maplines(sf::Lines);
// lines of cellings and flores
sf::VertexArray floorlines(sf::Lines);

void render()
{
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

            // calculat height of the wall
            wallHeight = screenHeight / distance;

            // colors of the floor
            sf::Color cell_color = sf::Color::White;
            cell_color.r /= distance;
            cell_color.g /= distance;
            cell_color.b /= distance;

            // colors of the ceiling
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
                                   sf::Color::Magenta));
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
            color.r /= 1.2;
            color.g /= 1.2;
            color.b /= 1.2;
        }

        // dynamic shadows on the walls (more dark color on distance)
        (color.r - (distance * 40)) > 0 ? color.r -= (distance * 40) : color.r = 0;
        (color.g - (distance * 40)) > 0 ? color.g -= (distance * 40) : color.g = 0;
        (color.b - (distance * 40)) > 0 ? color.b -= (distance * 40) : color.b = 0;

        // very basic dynamic ligthinig
        for (int i = -1; i < 2; i++)
        {
            for (int j = -1; j < 2; j++)
            {
                if ((getTile(mapPos.x + i, mapPos.y + j) == '5') && distance > 1)
                {
                    if (tile == '5')
                    {
                        color.r += (distance * 6);
                        color.g += (distance * 5);
                        color.b += (distance * 3);
                    }
                    else
                    {
                        color.r += distance * 6 * ((j == 1 || i == 1) ? wall_x : (1 - wall_x));
                        color.g += distance * 5 * ((j == 1 || i == 1) ? wall_x : (1 - wall_x));
                        color.b += distance * 3 * ((j == 1 || i == 1) ? wall_x : (1 - wall_x));
                    }
                }
            }
        }

        // add line to vertex buffer of walls
        lines.append(sf::Vertex(
            sf::Vector2f((float)x, (float)drawStart),
            color,
            sf::Vector2f((float)texture_coords.x, (float)texture_coords.y + 1)));
        lines.append(sf::Vertex(
            sf::Vector2f((float)x, (float)drawEnd),
            color,
            sf::Vector2f((float)texture_coords.x, (float)(texture_coords.y + texture_wall_size - 1))));
    }
}

sf::VertexArray getLines()
{
    return lines;
}

sf::VertexArray getMapLines()
{
    return maplines;
}

sf::VertexArray getFloorLines()
{
    return floorlines;
}

void clearAllLines()
{
    lines.clear();
    floorlines.clear();
    maplines.clear();
}