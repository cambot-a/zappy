/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the vision cone geometry
*/

#include <criterion/criterion.h>
#include <vector>

#include "server/game/VisionCone.hpp"

using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::VisionCone;

/* 1. level 0 returns the single player tile */

Test(vision_cone, level_zero_single_tile)
{
    const std::vector<Position> tiles =
        VisionCone::tilesFor(Position(5, 5), Orientation::NORTH, 0, 10, 10);
    cr_assert_eq(tiles.size(), 1U);
    cr_assert(tiles[0] == Position(5, 5));
}

/* 2. level 1 facing north */

Test(vision_cone, level_one_north)
{
    const std::vector<Position> tiles =
        VisionCone::tilesFor(Position(5, 5), Orientation::NORTH, 1, 10, 10);
    cr_assert_eq(tiles.size(), 4U);
    cr_assert(tiles[0] == Position(5, 5));
    cr_assert(tiles[1] == Position(4, 4));
    cr_assert(tiles[2] == Position(5, 4));
    cr_assert(tiles[3] == Position(6, 4));
}

/* 3. level 2 facing north */

Test(vision_cone, level_two_north)
{
    const std::vector<Position> tiles =
        VisionCone::tilesFor(Position(5, 5), Orientation::NORTH, 2, 10, 10);
    cr_assert_eq(tiles.size(), 9U);
    cr_assert(tiles[4] == Position(3, 3));
    cr_assert(tiles[6] == Position(5, 3));
    cr_assert(tiles[8] == Position(7, 3));
}

/* 4. level 1 facing east */

Test(vision_cone, level_one_east)
{
    const std::vector<Position> tiles =
        VisionCone::tilesFor(Position(5, 5), Orientation::EAST, 1, 10, 10);
    cr_assert_eq(tiles.size(), 4U);
    cr_assert(tiles[0] == Position(5, 5));
    cr_assert(tiles[1] == Position(6, 4));
    cr_assert(tiles[2] == Position(6, 5));
    cr_assert(tiles[3] == Position(6, 6));
}

/* 5. wrap at top edge */

Test(vision_cone, wrap_top_edge)
{
    const std::vector<Position> tiles =
        VisionCone::tilesFor(Position(5, 0), Orientation::NORTH, 1, 10, 10);
    cr_assert(tiles[0] == Position(5, 0));
    cr_assert(tiles[1] == Position(4, 9));
    cr_assert(tiles[2] == Position(5, 9));
    cr_assert(tiles[3] == Position(6, 9));
}

/* 6. wrap at left edge facing west */

Test(vision_cone, wrap_left_edge_west)
{
    const std::vector<Position> tiles =
        VisionCone::tilesFor(Position(0, 5), Orientation::WEST, 1, 10, 10);
    cr_assert(tiles[0] == Position(0, 5));
    cr_assert(tiles[1] == Position(9, 6));
    cr_assert(tiles[2] == Position(9, 5));
    cr_assert(tiles[3] == Position(9, 4));
}

/* 7. tile count is (level + 1) squared */

Test(vision_cone, count_formula)
{
    for (int level = 0; level <= 5; ++level) {
        const std::vector<Position> tiles = VisionCone::tilesFor(
            Position(5, 5), Orientation::NORTH, level, 10, 10);
        cr_assert_eq(tiles.size(),
            static_cast<std::size_t>((level + 1) * (level + 1)));
    }
}
