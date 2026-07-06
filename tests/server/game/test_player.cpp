/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for Player
*/

#include <criterion/criterion.h>

#include "server/game/Player.hpp"

using zappy::server::game::Orientation;
using zappy::server::game::Player;
using zappy::server::game::PlayerState;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;

/* construction */

Test(player, defaults)
{
    const Player p(1, "alpha", Position(2, 3), Orientation::NORTH);
    cr_assert_eq(p.id(), 1);
    cr_assert_str_eq(p.team().c_str(), "alpha");
    cr_assert(p.position() == Position(2, 3));
    cr_assert_eq(static_cast<int>(p.orientation()), static_cast<int>(Orientation::NORTH));
    cr_assert_eq(p.level(), 1);
    cr_assert_eq(p.resource(ResourceType::FOOD), 10);
    cr_assert_eq(p.resource(ResourceType::LINEMATE), 0);
    cr_assert_eq(static_cast<int>(p.state()), static_cast<int>(PlayerState::ALIVE));
}

/* setters */

Test(player, setters)
{
    Player p(1, "alpha", Position(0, 0), Orientation::NORTH);
    p.setPosition(Position(4, 5));
    p.setOrientation(Orientation::WEST);
    p.setLevel(3);
    p.setState(PlayerState::DEAD);
    cr_assert(p.position() == Position(4, 5));
    cr_assert_eq(static_cast<int>(p.orientation()), static_cast<int>(Orientation::WEST));
    cr_assert_eq(p.level(), 3);
    cr_assert_eq(static_cast<int>(p.state()), static_cast<int>(PlayerState::DEAD));
}

/* inventory */

Test(player, add_resource)
{
    Player p(1, "alpha", Position(0, 0), Orientation::NORTH);
    cr_assert(p.addResource(ResourceType::SIBUR, 2));
    cr_assert_eq(p.resource(ResourceType::SIBUR), 2);
}

Test(player, add_negative_rejected)
{
    Player p(1, "alpha", Position(0, 0), Orientation::NORTH);
    cr_assert_not(p.addResource(ResourceType::SIBUR, -1));
    cr_assert_eq(p.resource(ResourceType::SIBUR), 0);
}

Test(player, remove_resource_success)
{
    Player p(1, "alpha", Position(0, 0), Orientation::NORTH);
    cr_assert(p.removeResource(ResourceType::FOOD, 4));
    cr_assert_eq(p.resource(ResourceType::FOOD), 6);
}

Test(player, remove_too_much_fails)
{
    Player p(1, "alpha", Position(0, 0), Orientation::NORTH);
    cr_assert_not(p.removeResource(ResourceType::LINEMATE, 1));
    cr_assert_eq(p.resource(ResourceType::LINEMATE), 0);
}
