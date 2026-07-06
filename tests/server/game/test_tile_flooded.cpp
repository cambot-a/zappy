/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the Tile flood flag (ZAP-50, bonus)
*/

#include <criterion/criterion.h>

#include "server/game/Tile.hpp"

using zappy::server::game::Tile;

Test(tile_flooded, default_is_not_flooded)
{
    const Tile t;
    cr_assert_not(t.isFlooded());
}

Test(tile_flooded, set_flooded_true)
{
    Tile t;
    t.setFlooded(true);
    cr_assert(t.isFlooded());
}

Test(tile_flooded, set_flooded_false_flips_back)
{
    Tile t;
    t.setFlooded(true);
    t.setFlooded(false);
    cr_assert_not(t.isFlooded());
}
