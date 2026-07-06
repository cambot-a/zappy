/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Toroidal map coordinate value type
*/

#include "server/game/Position.hpp"

/**
 * @brief Component-wise sum, result is not wrapped.
 *
 * @param rhs right-hand operand
 * @return Position the unwrapped sum
 */
zappy::server::game::Position
zappy::server::game::Position::operator+(const Position &rhs) const noexcept
{
    return Position(_x + rhs._x, _y + rhs._y);
}

/**
 * @brief Component-wise difference, result is not wrapped.
 *
 * @param rhs right-hand operand
 * @return Position the unwrapped difference
 */
zappy::server::game::Position
zappy::server::game::Position::operator-(const Position &rhs) const noexcept
{
    return Position(_x - rhs._x, _y - rhs._y);
}

/**
 * @brief Equality on both coordinates.
 *
 * @param rhs right-hand operand
 * @return bool true if both coordinates match
 */
bool zappy::server::game::Position::operator==(
    const Position &rhs) const noexcept
{
    return _x == rhs._x && _y == rhs._y;
}

/**
 * @brief Inequality on either coordinate.
 *
 * @param rhs right-hand operand
 * @return bool true if any coordinate differs
 */
bool zappy::server::game::Position::operator!=(
    const Position &rhs) const noexcept
{
    return !(*this == rhs);
}

/**
 * @brief Wrap both coordinates into [0, width) and [0, height).
 *
 * @param width map width
 * @param height map height
 * @return Position the normalized position
 */
zappy::server::game::Position
zappy::server::game::Position::normalized(int width, int height) const noexcept
{
    const int nx = ((_x % width) + width) % width;
    const int ny = ((_y % height) + height) % height;

    return Position(nx, ny);
}

/**
 * @brief Shortest dx/dy vector from this to other on the torus.
 *
 * @param other destination position
 * @param width map width
 * @param height map height
 * @return Position the shortest vector
 */
zappy::server::game::Position
zappy::server::game::Position::shortestVectorTo(
    const Position &other, int width, int height) const noexcept
{
    const Position from = normalized(width, height);
    const Position to = other.normalized(width, height);
    const int dx = wrapDelta(to._x - from._x, width);
    const int dy = wrapDelta(to._y - from._y, height);

    return Position(dx, dy);
}

/**
 * @brief Fold a raw delta into the shortest signed offset on one axis.
 *
 * @param raw unwrapped delta
 * @param size axis length
 * @return int the shortest signed delta
 */
int zappy::server::game::Position::wrapDelta(int raw, int size) noexcept
{
    const int wrapped = ((raw % size) + size) % size;
    const int result = (wrapped > size / 2) ? wrapped - size : wrapped;

    return result;
}
