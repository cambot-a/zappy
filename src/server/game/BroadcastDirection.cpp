/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** BroadcastDirection implementation
*/

#include <array>
#include <cmath>
#include <cstddef>

#include "server/game/BroadcastDirection.hpp"

namespace {
constexpr double PI = 3.141592653589793;
constexpr int SECTOR_COUNT = 8;
}

/**
 * @brief Direction the sender appears in, from the receiver's POV.
 *
 * @param senderPos tile of the broadcasting player
 * @param receiverPos tile of the listening player
 * @param receiverOrient facing of the listening player
 * @param worldWidth map width for toroidal wrapping
 * @param worldHeight map height for toroidal wrapping
 * @return int the K direction (0 same tile, 1..8 clockwise from front)
 */
int zappy::server::game::BroadcastDirection::compute(Position senderPos,
    Position receiverPos, Orientation receiverOrient, int worldWidth,
    int worldHeight) noexcept
{
    const Position delta =
        receiverPos.shortestVectorTo(senderPos, worldWidth, worldHeight);
    int k = 0;

    if (delta.x() != 0 || delta.y() != 0) {
        const double worldAngle = std::atan2(
            static_cast<double>(delta.y()), static_cast<double>(delta.x()));
        double relative = worldAngle - orientationAngle(receiverOrient);
        while (relative < 0)
            relative += 2 * PI;
        while (relative >= 2 * PI)
            relative -= 2 * PI;
        k = static_cast<int>(std::round(relative * 4 / PI)) % SECTOR_COUNT + 1;
    }
    return k;
}

/**
 * @brief World-space angle of the receiver's front, in radians.
 *
 * @param o the receiver's facing
 * @return double the front angle (atan2 convention)
 */
double zappy::server::game::BroadcastDirection::orientationAngle(
    Orientation o) noexcept
{
    static constexpr std::array<double, 5> ANGLES = {{
        0.0,
        -PI / 2,
        0.0,
        PI / 2,
        PI
    }};

    return ANGLES[static_cast<std::size_t>(o)];
}
