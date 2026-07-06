/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Orientation math: forward delta and clockwise/counter-clockwise rotation
*/

#ifndef SERVER_GAME_ORIENTATIONHELPER_HPP_
    #define SERVER_GAME_ORIENTATIONHELPER_HPP_

    #include <array>
    #include <cstddef>

    #include "server/game/Constants.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {

/**
 * @brief Constexpr orientation lookups.
 *
 * Coordinate convention: NORTH = -y, EAST = +x, SOUTH = +y, WEST = -x.
 * Right turns clockwise (N->E->S->W), Left counter-clockwise (N->W->S->E).
 * Tables are indexed by static_cast<size_t>(Orientation); index 0 is unused
 * since Orientation values run 1..4.
 */
class OrientationHelper {
public:
    /**
     * @brief One-tile move vector for facing @p o.
     *
     * @param o the facing
     * @return Position the (dx, dy) delta
     */
    [[nodiscard]] static constexpr Position forwardDelta(Orientation o) noexcept
    {
        return DELTAS[static_cast<std::size_t>(o)];
    }

    /**
     * @brief Move vector to the player's right for facing @p o.
     *
     * @param o the facing
     * @return Position the (dx, dy) delta
     */
    [[nodiscard]] static constexpr Position rightDelta(Orientation o) noexcept
    {
        return forwardDelta(rotateRight(o));
    }

    /**
     * @brief Facing after a clockwise quarter turn.
     *
     * @param o the current facing
     * @return Orientation the rotated facing
     */
    [[nodiscard]] static constexpr Orientation rotateRight(Orientation o)
        noexcept
    {
        return NEXT_RIGHT[static_cast<std::size_t>(o)];
    }

    /**
     * @brief Facing after a counter-clockwise quarter turn.
     *
     * @param o the current facing
     * @return Orientation the rotated facing
     */
    [[nodiscard]] static constexpr Orientation rotateLeft(Orientation o)
        noexcept
    {
        return NEXT_LEFT[static_cast<std::size_t>(o)];
    }

private:
    static constexpr std::array<Position, 5> DELTAS = {{
        {0, 0},
        {0, -1},
        {1, 0},
        {0, 1},
        {-1, 0}
    }};
    static constexpr std::array<Orientation, 5> NEXT_RIGHT = {{
        Orientation::NORTH,
        Orientation::EAST,
        Orientation::SOUTH,
        Orientation::WEST,
        Orientation::NORTH
    }};
    static constexpr std::array<Orientation, 5> NEXT_LEFT = {{
        Orientation::NORTH,
        Orientation::WEST,
        Orientation::NORTH,
        Orientation::EAST,
        Orientation::SOUTH
    }};
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_ORIENTATIONHELPER_HPP_ */
