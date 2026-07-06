/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for Tile
*/

#include <criterion/criterion.h>

#include "server/game/Tile.hpp"

using zappy::server::game::ResourceType;
using zappy::server::game::Tile;

/* resources */

Test(tile, resources_start_empty)
{
    const Tile t;
    cr_assert_eq(t.resource(ResourceType::FOOD), 0);
    cr_assert_eq(t.resource(ResourceType::THYSTAME), 0);
}

Test(tile, set_resource)
{
    Tile t;
    t.setResource(ResourceType::LINEMATE, 5);
    cr_assert_eq(t.resource(ResourceType::LINEMATE), 5);
}

Test(tile, set_negative_clamps_to_zero)
{
    Tile t;
    t.setResource(ResourceType::SIBUR, -3);
    cr_assert_eq(t.resource(ResourceType::SIBUR), 0);
}

Test(tile, add_resource)
{
    Tile t;
    cr_assert(t.addResource(ResourceType::FOOD, 4));
    cr_assert(t.addResource(ResourceType::FOOD, 2));
    cr_assert_eq(t.resource(ResourceType::FOOD), 6);
}

Test(tile, add_negative_rejected)
{
    Tile t;
    cr_assert_not(t.addResource(ResourceType::FOOD, -1));
    cr_assert_eq(t.resource(ResourceType::FOOD), 0);
}

Test(tile, remove_resource_success)
{
    Tile t;
    t.setResource(ResourceType::MENDIANE, 5);
    cr_assert(t.removeResource(ResourceType::MENDIANE, 3));
    cr_assert_eq(t.resource(ResourceType::MENDIANE), 2);
}

Test(tile, remove_too_much_fails)
{
    Tile t;
    t.setResource(ResourceType::PHIRAS, 2);
    cr_assert_not(t.removeResource(ResourceType::PHIRAS, 3));
    cr_assert_eq(t.resource(ResourceType::PHIRAS), 2);
}

Test(tile, remove_negative_rejected)
{
    Tile t;
    t.setResource(ResourceType::PHIRAS, 2);
    cr_assert_not(t.removeResource(ResourceType::PHIRAS, -1));
    cr_assert_eq(t.resource(ResourceType::PHIRAS), 2);
}

/* player/egg list */

Test(tile, add_and_list_players)
{
    Tile t;
    t.addPlayer(7);
    t.addPlayer(9);
    cr_assert_eq(t.playerIds().size(), 2U);
    cr_assert_eq(t.playerIds()[0], 7);
    cr_assert_eq(t.playerIds()[1], 9);
}

Test(tile, remove_player)
{
    Tile t;
    t.addPlayer(7);
    t.addPlayer(9);
    t.removePlayer(7);
    cr_assert_eq(t.playerIds().size(), 1U);
    cr_assert_eq(t.playerIds()[0], 9);
}

Test(tile, remove_absent_player_is_noop)
{
    Tile t;
    t.addPlayer(7);
    t.removePlayer(42);
    cr_assert_eq(t.playerIds().size(), 1U);
}

Test(tile, add_and_remove_eggs)
{
    Tile t;
    t.addEgg(1);
    t.addEgg(2);
    t.removeEgg(1);
    cr_assert_eq(t.eggIds().size(), 1U);
    cr_assert_eq(t.eggIds()[0], 2);
}
