/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** VisionCone implementation
*/

#include "server/game/VisionCone.hpp"
#include "server/game/OrientationHelper.hpp"

/**
 * @brief Vision tiles for a player, nearest row first, left to right.
 *
 * @param playerPos the player's tile
 * @param orient the player's facing
 * @param level the player's elevation level
 * @param worldWidth map width for wrapping
 * @param worldHeight map height for wrapping
 * @return std::vector<Position> the (level+1)^2 normalized tiles
 */
std::vector<zappy::server::game::Position>
zappy::server::game::VisionCone::tilesFor(Position playerPos,
    Orientation orient, int level, int worldWidth, int worldHeight)
{
    std::vector<Position> tiles;
    tiles.reserve(static_cast<std::size_t>((level + 1) * (level + 1)));
    const Position forward = OrientationHelper::forwardDelta(orient);
    const Position right = OrientationHelper::rightDelta(orient);
    for (int r = 0; r <= level; ++r) {
        for (int c = -r; c <= r; ++c) {
            const int x = playerPos.x() + forward.x() * r + right.x() * c;
            const int y = playerPos.y() + forward.y() * r + right.y() * c;
            tiles.push_back(
                Position(x, y).normalized(worldWidth, worldHeight));
        }
    }
    return tiles;
}
