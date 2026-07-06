/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the Biome table (ZAP-52, bonus)
*/

#include <criterion/criterion.h>

#include "server/game/Biome.hpp"

using zappy::server::game::Biome;
using zappy::server::game::biomeInfoFor;
using zappy::server::game::ResourceType;
using zappy::server::game::BIOME_COUNT;

Test(biome, table_has_six_entries)
{
    cr_assert_eq(BIOME_COUNT, 6U);
}

Test(biome, plain_is_traversable)
{
    cr_assert(biomeInfoFor(Biome::PLAIN).traversable);
}

Test(biome, mountain_is_not_traversable)
{
    cr_assert_not(biomeInfoFor(Biome::MOUNTAIN).traversable);
}

Test(biome, peak_is_not_traversable)
{
    cr_assert_not(biomeInfoFor(Biome::PEAK).traversable);
}

Test(biome, plain_multipliers_are_all_one)
{
    const auto &info = biomeInfoFor(Biome::PLAIN);
    for (std::size_t i = 0; i < zappy::server::game::RESOURCE_COUNT; i++)
        cr_assert_float_eq(info.densityMultipliers[i], 1.0, 1e-9);
}

Test(biome, snow_plain_food_multiplier_is_half)
{
    const auto &info = biomeInfoFor(Biome::SNOW_PLAIN);
    const auto food = static_cast<std::size_t>(ResourceType::FOOD);
    cr_assert_float_eq(info.densityMultipliers[food], 0.5, 1e-9);
}

Test(biome, peak_has_highest_thystame_multiplier)
{
    const auto &info = biomeInfoFor(Biome::PEAK);
    const auto thystame = static_cast<std::size_t>(ResourceType::THYSTAME);
    cr_assert_float_eq(info.densityMultipliers[thystame], 3.0, 1e-9);
}

Test(biome, names_match)
{
    cr_assert_str_eq(std::string(biomeInfoFor(Biome::VALLEY).name).c_str(),
        "valley");
    cr_assert_str_eq(std::string(biomeInfoFor(Biome::SNOW_PLAIN).name).c_str(),
        "snow_plain");
}
