/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Toroidal map coordinate value type
*/

#ifndef SERVER_GAME_POSITION_HPP_
    #define SERVER_GAME_POSITION_HPP_

namespace zappy::server::game {

/**
 * @brief A (x, y) point on the toroidal map.
 */
class Position {
public:
    constexpr Position() noexcept : _x(0), _y(0) {}

    /**
     * @brief Build a position from explicit coordinates.
     *
     * @param x horizontal coordinate
     * @param y vertical coordinate
     */
    constexpr Position(int x, int y) noexcept : _x(x), _y(y) {}

    /**
     * @brief Horizontal coordinate.
     *
     * @return int the x value
     */
    [[nodiscard]] constexpr int x() const noexcept { return _x; }

    /**
     * @brief Vertical coordinate.
     *
     * @return int the y value
     */
    [[nodiscard]] constexpr int y() const noexcept { return _y; }

    /**
     * @brief Component-wise sum; result is not wrapped.
     *
     * @param rhs right-hand operand
     * @return Position the unwrapped sum
     */
    Position operator+(const Position &rhs) const noexcept;

    /**
     * @brief Component-wise difference; result is not wrapped.
     *
     * @param rhs right-hand operand
     * @return Position the unwrapped difference
     */
    Position operator-(const Position &rhs) const noexcept;

    /**
     * @brief Equality on both coordinates.
     *
     * @param rhs right-hand operand
     * @return bool true if both coordinates match
     */
    bool operator==(const Position &rhs) const noexcept;

    /**
     * @brief Inequality on either coordinate.
     *
     * @param rhs right-hand operand
     * @return bool true if any coordinate differs
     */
    bool operator!=(const Position &rhs) const noexcept;

    /**
     * @brief Wrap both coordinates into [0, width) and [0, height).
     *
     * @param width map width
     * @param height map height
     * @return Position the normalized position
     */
    [[nodiscard]] Position normalized(int width, int height) const noexcept;

    /**
     * @brief Shortest dx/dy vector from this to other on the torus.
     *
     * @param other destination position
     * @param width map width
     * @param height map height
     * @return Position the shortest vector
     */
    [[nodiscard]] Position shortestVectorTo(const Position &other,
        int width, int height) const noexcept;

private:
    /**
     * @brief Fold a raw delta into the shortest signed offset on one axis.
     *
     * @param raw unwrapped delta
     * @param size axis length
     * @return int the shortest signed delta
     */
    [[nodiscard]] static int wrapDelta(int raw, int size) noexcept;

    int _x;
    int _y;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_POSITION_HPP_ */
