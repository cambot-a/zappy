/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Pure geometry of a player's vision cone on the toroidal map
*/

#ifndef SERVER_GAME_VISIONCONE_HPP_
    #define SERVER_GAME_VISIONCONE_HPP_

    #include <vector>

    #include "server/game/Constants.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {

/**
 * @brief Computes the tiles a player sees, ordered as the Look protocol.
 */
class VisionCone {
public:
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
    [[nodiscard]] static std::vector<Position> tilesFor(
        Position playerPos, Orientation orient, int level,
        int worldWidth, int worldHeight);
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_VISIONCONE_HPP_ */
