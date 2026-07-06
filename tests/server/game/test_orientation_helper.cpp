/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the orientation helper
*/

#include <criterion/criterion.h>

#include "server/game/OrientationHelper.hpp"

using zappy::server::game::Orientation;
using zappy::server::game::OrientationHelper;
using zappy::server::game::Position;

/* 1. forward deltas match the coordinate convention */

Test(orientation_helper, forward_delta_north_is_minus_y)
{
    const Position d = OrientationHelper::forwardDelta(Orientation::NORTH);
    cr_assert_eq(d.x(), 0);
    cr_assert_eq(d.y(), -1);
}

Test(orientation_helper, forward_delta_east_is_plus_x)
{
    const Position d = OrientationHelper::forwardDelta(Orientation::EAST);
    cr_assert_eq(d.x(), 1);
    cr_assert_eq(d.y(), 0);
}

Test(orientation_helper, forward_delta_south_is_plus_y)
{
    const Position d = OrientationHelper::forwardDelta(Orientation::SOUTH);
    cr_assert_eq(d.x(), 0);
    cr_assert_eq(d.y(), 1);
}

Test(orientation_helper, forward_delta_west_is_minus_x)
{
    const Position d = OrientationHelper::forwardDelta(Orientation::WEST);
    cr_assert_eq(d.x(), -1);
    cr_assert_eq(d.y(), 0);
}

/* 2. clockwise rotation cycles N -> E -> S -> W -> N */

Test(orientation_helper, rotate_right_cycles_clockwise)
{
    Orientation o = Orientation::NORTH;
    o = OrientationHelper::rotateRight(o);
    cr_assert_eq(o, Orientation::EAST);
    o = OrientationHelper::rotateRight(o);
    cr_assert_eq(o, Orientation::SOUTH);
    o = OrientationHelper::rotateRight(o);
    cr_assert_eq(o, Orientation::WEST);
    o = OrientationHelper::rotateRight(o);
    cr_assert_eq(o, Orientation::NORTH);
}

/* 3. counter-clockwise rotation cycles N -> W -> S -> E -> N */

Test(orientation_helper, rotate_left_cycles_counter_clockwise)
{
    Orientation o = Orientation::NORTH;
    o = OrientationHelper::rotateLeft(o);
    cr_assert_eq(o, Orientation::WEST);
    o = OrientationHelper::rotateLeft(o);
    cr_assert_eq(o, Orientation::SOUTH);
    o = OrientationHelper::rotateLeft(o);
    cr_assert_eq(o, Orientation::EAST);
    o = OrientationHelper::rotateLeft(o);
    cr_assert_eq(o, Orientation::NORTH);
}

/* 4. right then left is identity for every orientation */

Test(orientation_helper, right_then_left_round_trips)
{
    const Orientation all[] = {Orientation::NORTH, Orientation::EAST,
        Orientation::SOUTH, Orientation::WEST};

    for (const Orientation o : all)
        cr_assert_eq(OrientationHelper::rotateRight(
            OrientationHelper::rotateLeft(o)), o);
}
