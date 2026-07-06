/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Sound-direction (K) computation for the Broadcast command
*/

#ifndef SERVER_GAME_BROADCASTDIRECTION_HPP_
    #define SERVER_GAME_BROADCASTDIRECTION_HPP_

    #include "server/game/Constants.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {

/**
 * @brief Pure math for the direction a broadcast appears to come from.
 *
 * K=0 is the receiver's own tile; 1..8 number the surrounding tiles
 * clockwise starting in front of the receiver, relative to its facing.
 */
class BroadcastDirection {
public:
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
    [[nodiscard]] static int compute(Position senderPos, Position receiverPos,
        Orientation receiverOrient, int worldWidth, int worldHeight) noexcept;

private:
    /**
     * @brief World-space angle of the receiver's front, in radians.
     *
     * @param o the receiver's facing
     * @return double the front angle (atan2 convention)
     */
    [[nodiscard]] static double orientationAngle(Orientation o) noexcept;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_BROADCASTDIRECTION_HPP_ */
