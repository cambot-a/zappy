/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for BroadcastDirection (K sound-direction math)
*/

#include <criterion/criterion.h>

#include "server/game/BroadcastDirection.hpp"
#include "server/game/Constants.hpp"
#include "server/game/Position.hpp"

using zappy::server::game::BroadcastDirection;
using zappy::server::game::Orientation;
using zappy::server::game::Position;

namespace {

constexpr int W = 10;
constexpr int H = 10;

int k(Position sender, Position receiver, Orientation orient)
{
    return BroadcastDirection::compute(sender, receiver, orient, W, H);
}

} // namespace

/* 1. sender on the receiver's own tile yields K=0 regardless of facing */

Test(broadcast_direction, same_tile)
{
    cr_assert_eq(k(Position(5, 5), Position(5, 5), Orientation::NORTH), 0);
    cr_assert_eq(k(Position(5, 5), Position(5, 5), Orientation::EAST), 0);
    cr_assert_eq(k(Position(5, 5), Position(5, 5), Orientation::SOUTH), 0);
    cr_assert_eq(k(Position(5, 5), Position(5, 5), Orientation::WEST), 0);
}

/* 2. NORTH-facing receiver: the eight surrounding tiles map clockwise */

Test(broadcast_direction, north_eight_sectors)
{
    const Position r(5, 5);
    cr_assert_eq(k(Position(5, 4), r, Orientation::NORTH), 1);
    cr_assert_eq(k(Position(6, 4), r, Orientation::NORTH), 2);
    cr_assert_eq(k(Position(6, 5), r, Orientation::NORTH), 3);
    cr_assert_eq(k(Position(6, 6), r, Orientation::NORTH), 4);
    cr_assert_eq(k(Position(5, 6), r, Orientation::NORTH), 5);
    cr_assert_eq(k(Position(4, 6), r, Orientation::NORTH), 6);
    cr_assert_eq(k(Position(4, 5), r, Orientation::NORTH), 7);
    cr_assert_eq(k(Position(4, 4), r, Orientation::NORTH), 8);
}

/* 3. EAST-facing receiver: front rotates to the east */

Test(broadcast_direction, east_sectors)
{
    const Position r(5, 5);
    cr_assert_eq(k(Position(6, 5), r, Orientation::EAST), 1);
    cr_assert_eq(k(Position(6, 6), r, Orientation::EAST), 2);
    cr_assert_eq(k(Position(5, 6), r, Orientation::EAST), 3);
    cr_assert_eq(k(Position(4, 5), r, Orientation::EAST), 5);
}

/* 4. SOUTH and WEST coverage: one front tile each */

Test(broadcast_direction, south_west_fronts)
{
    const Position r(5, 5);
    cr_assert_eq(k(Position(5, 6), r, Orientation::SOUTH), 1);
    cr_assert_eq(k(Position(4, 5), r, Orientation::WEST), 1);
}

/* 5. wrap: the shortest vector crosses the torus edge */

Test(broadcast_direction, torus_wrap_front)
{
    cr_assert_eq(k(Position(0, 5), Position(9, 5), Orientation::EAST), 1);
}

/* 6. far diagonal: atan2 keeps the sector independent of distance */

Test(broadcast_direction, far_diagonal)
{
    cr_assert_eq(k(Position(8, 2), Position(5, 5), Orientation::NORTH), 2);
}
