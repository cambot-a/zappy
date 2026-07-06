/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Tests for the elevation requirement table
*/

#include <criterion/criterion.h>

#include "server/game/Constants.hpp"
#include "server/game/ElevationRules.hpp"

using zappy::server::game::ElevationRules;
using zappy::server::game::ResourceType;

namespace {

int stone(const zappy::server::game::ElevationRule &rule, ResourceType type)
{
    return rule.stonesRequired[static_cast<std::size_t>(type)];
}

} // namespace

Test(elevation_rules, level_one_needs_one_player_one_linemate)
{
    const auto &rule = ElevationRules::forLevel(1);
    cr_assert_eq(rule.playersRequired, 1);
    cr_assert_eq(stone(rule, ResourceType::LINEMATE), 1);
    cr_assert_eq(stone(rule, ResourceType::DERAUMERE), 0);
}

Test(elevation_rules, level_seven_full_recipe)
{
    const auto &rule = ElevationRules::forLevel(7);
    cr_assert_eq(rule.playersRequired, 6);
    cr_assert_eq(stone(rule, ResourceType::LINEMATE), 2);
    cr_assert_eq(stone(rule, ResourceType::DERAUMERE), 2);
    cr_assert_eq(stone(rule, ResourceType::SIBUR), 2);
    cr_assert_eq(stone(rule, ResourceType::MENDIANE), 2);
    cr_assert_eq(stone(rule, ResourceType::PHIRAS), 2);
    cr_assert_eq(stone(rule, ResourceType::THYSTAME), 1);
}

Test(elevation_rules, level_eight_is_zeros)
{
    const auto &rule = ElevationRules::forLevel(8);
    cr_assert_eq(rule.playersRequired, 0);
    cr_assert_eq(stone(rule, ResourceType::LINEMATE), 0);
}

Test(elevation_rules, level_zero_is_zeros)
{
    const auto &rule = ElevationRules::forLevel(0);
    cr_assert_eq(rule.playersRequired, 0);
}

Test(elevation_rules, negative_level_is_zeros)
{
    const auto &rule = ElevationRules::forLevel(-1);
    cr_assert_eq(rule.playersRequired, 0);
}

Test(elevation_rules, max_level_is_eight)
{
    cr_assert_eq(ElevationRules::maxLevel(), 8);
}
