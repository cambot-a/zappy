/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the Tile biome attribute (ZAP-52, bonus)
*/

#include <criterion/criterion.h>

#include "server/game/Biome.hpp"
#include "server/game/Tile.hpp"

using zappy::server::game::Biome;
using zappy::server::game::Tile;

Test(tile_biome, default_is_plain)
{
    const Tile t;
    cr_assert_eq(t.biome(), Biome::PLAIN);
}

Test(tile_biome, set_biome_mountain)
{
    Tile t;
    t.setBiome(Biome::MOUNTAIN);
    cr_assert_eq(t.biome(), Biome::MOUNTAIN);
}

Test(tile_biome, set_biome_roundtrip)
{
    Tile t;
    t.setBiome(Biome::PEAK);
    cr_assert_eq(t.biome(), Biome::PEAK);
    t.setBiome(Biome::PLAIN);
    cr_assert_eq(t.biome(), Biome::PLAIN);
}
