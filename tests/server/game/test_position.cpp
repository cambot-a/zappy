/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for Position
*/

#include <criterion/criterion.h>

#include "server/game/Position.hpp"

using zappy::server::game::Position;

/*  construction/getters */

Test(position, default_is_origin)
{
    const Position p;
    cr_assert_eq(p.x(), 0);
    cr_assert_eq(p.y(), 0);
}

Test(position, explicit_construction)
{
    const Position p(3, 7);
    cr_assert_eq(p.x(), 3);
    cr_assert_eq(p.y(), 7);
}

/* equality checks */

Test(position, equality_true)
{
    cr_assert(Position(2, 5) == Position(2, 5));
}

Test(position, equality_false)
{
    cr_assert_not(Position(2, 5) == Position(2, 6));
}

Test(position, inequality)
{
    cr_assert(Position(2, 5) != Position(9, 5));
    cr_assert_not(Position(2, 5) != Position(2, 5));
}

/* arithmetic */

Test(position, addition_no_wrap)
{
    const Position r = Position(3, 4) + Position(5, 6);
    cr_assert_eq(r.x(), 8);
    cr_assert_eq(r.y(), 10);
}

Test(position, subtraction_no_wrap)
{
    const Position r = Position(3, 4) - Position(5, 6);
    cr_assert_eq(r.x(), -2);
    cr_assert_eq(r.y(), -2);
}

/* normalized */

Test(position, normalized_in_range_unchanged)
{
    const Position r = Position(3, 4).normalized(10, 10);
    cr_assert_eq(r.x(), 3);
    cr_assert_eq(r.y(), 4);
}

Test(position, normalized_positive_overflow)
{
    const Position r = Position(15, 12).normalized(10, 10);
    cr_assert_eq(r.x(), 5);
    cr_assert_eq(r.y(), 2);
}

Test(position, normalized_negative_wraps)
{
    const Position r = Position(-1, -1).normalized(10, 10);
    cr_assert_eq(r.x(), 9);
    cr_assert_eq(r.y(), 9);
}

Test(position, normalized_large_negative)
{
    const Position r = Position(-25, -25).normalized(10, 10);
    cr_assert_eq(r.x(), 5);
    cr_assert_eq(r.y(), 5);
}

Test(position, normalized_zero_stays_zero)
{
    const Position r = Position(0, 0).normalized(10, 10);
    cr_assert_eq(r.x(), 0);
    cr_assert_eq(r.y(), 0);
}

/* shortestVectorTo */

Test(position, shortest_same_point)
{
    const Position v = Position(5, 5).shortestVectorTo(Position(5, 5), 10, 10);
    cr_assert_eq(v.x(), 0);
    cr_assert_eq(v.y(), 0);
}

Test(position, shortest_adjacent_points)
{
    const Position base(5, 5);
    cr_assert_eq(base.shortestVectorTo(Position(6, 5), 10, 10).x(), 1);
    cr_assert_eq(base.shortestVectorTo(Position(5, 6), 10, 10).y(), 1);
    cr_assert_eq(base.shortestVectorTo(Position(4, 5), 10, 10).x(), -1);
    cr_assert_eq(base.shortestVectorTo(Position(5, 4), 10, 10).y(), -1);
}

Test(position, shortest_crosses_right_edge)
{
    const Position v = Position(9, 5).shortestVectorTo(Position(0, 5), 10, 10);
    cr_assert_eq(v.x(), 1);
    cr_assert_eq(v.y(), 0);
}

Test(position, shortest_crosses_bottom_edge)
{
    const Position v = Position(5, 9).shortestVectorTo(Position(5, 0), 10, 10);
    cr_assert_eq(v.x(), 0);
    cr_assert_eq(v.y(), 1);
}

Test(position, shortest_diagonal_cross_edge)
{
    const Position v = Position(9, 9).shortestVectorTo(Position(0, 0), 10, 10);
    cr_assert_eq(v.x(), 1);
    cr_assert_eq(v.y(), 1);
}

Test(position, shortest_opposite_even_picks_positive)
{
    const Position v = Position(0, 0).shortestVectorTo(Position(5, 5), 10, 10);
    cr_assert_eq(v.x(), 5);
    cr_assert_eq(v.y(), 5);
}

Test(position, shortest_normalizes_unnormalized_inputs)
{
    const Position v = Position(19, 5).shortestVectorTo(Position(10, 5), 10, 10);
    cr_assert_eq(v.x(), 1);
    cr_assert_eq(v.y(), 0);
}
