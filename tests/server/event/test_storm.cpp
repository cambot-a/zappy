/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the Storm natural event (ZAP-49, bonus)
*/

#include <criterion/criterion.h>
#include <string>

#include "server/event/Storm.hpp"
#include "server/game/Constants.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::event::Storm;
using zappy::server::game::Orientation;
using zappy::server::game::PlayerState;
using zappy::server::game::Position;
using zappy::server::game::World;

namespace {

struct StormFixture {
    World world{10, 10, {"team"}, 8};

    int spawnPlayer(Position pos, Orientation orient = Orientation::NORTH)
    {
        return world.addPlayer("team", pos, orient);
    }

    Position positionOf(int id) const
    {
        return world.player(id).position();
    }

    void step(Storm &storm)
    {
        static_cast<void>(storm.applyTick(world));
    }
};

} // namespace

Test(storm, zero_duration_expires_immediately)
{
    StormFixture fx;
    Storm storm(Position(5, 5), 3, Orientation::EAST, 1, 0);

    cr_assert(!storm.applyTick(fx.world));
}

Test(storm, duration_five_stays_active_for_five_ticks)
{
    StormFixture fx;
    Storm storm(Position(5, 5), 3, Orientation::EAST, 99, 5);

    for (int i = 0; i < 5; i++)
        cr_assert(storm.applyTick(fx.world));
    cr_assert(!storm.applyTick(fx.world));
}

Test(storm, push_interval_one_pushes_every_tick)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(5, 5));
    Storm storm(Position(5, 5), 3, Orientation::EAST, 1, 5);

    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(6, 5));
    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(7, 5));
}

Test(storm, push_interval_two_pushes_every_two_ticks)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(5, 5));
    Storm storm(Position(5, 5), 3, Orientation::EAST, 2, 5);

    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(5, 5));
    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(6, 5));
    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(6, 5));
    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(7, 5));
}

Test(storm, push_wraps_on_torus)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(9, 5));
    Storm storm(Position(9, 5), 3, Orientation::EAST, 1, 5);

    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(0, 5));
}

Test(storm, player_outside_zone_is_not_pushed)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(0, 0));
    Storm storm(Position(5, 5), 1, Orientation::EAST, 1, 5);

    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(0, 0));
}

Test(storm, dead_player_is_not_pushed)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(5, 5));
    Storm storm(Position(5, 5), 3, Orientation::EAST, 1, 5);

    fx.world.killPlayer(id);
    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(5, 5));
    cr_assert(fx.world.player(id).state() == PlayerState::DEAD);
}

Test(storm, incanting_player_is_pushed)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(5, 5));
    Storm storm(Position(5, 5), 3, Orientation::EAST, 1, 5);

    fx.world.freezePlayer(id);
    fx.step(storm);
    cr_assert(fx.positionOf(id) == Position(6, 5));
    cr_assert(fx.world.player(id).state() == PlayerState::INCANTING);
}

Test(storm, multiple_players_in_zone_all_pushed_same_direction)
{
    StormFixture fx;
    const int a = fx.spawnPlayer(Position(4, 5), Orientation::NORTH);
    const int b = fx.spawnPlayer(Position(5, 5), Orientation::SOUTH);
    const int c = fx.spawnPlayer(Position(6, 5), Orientation::WEST);
    Storm storm(Position(5, 5), 3, Orientation::EAST, 1, 5);

    fx.step(storm);
    cr_assert(fx.positionOf(a) == Position(5, 5));
    cr_assert(fx.positionOf(b) == Position(6, 5));
    cr_assert(fx.positionOf(c) == Position(7, 5));
}

Test(storm, zone_membership_respects_toroidal_distance)
{
    StormFixture fx;
    const int id = fx.spawnPlayer(Position(9, 9));
    Storm tightStorm(Position(1, 1), 2, Orientation::EAST, 1, 5);

    fx.step(tightStorm);
    cr_assert(fx.positionOf(id) == Position(9, 9));

    Storm wideStorm(Position(1, 1), 5, Orientation::EAST, 1, 5);
    fx.step(wideStorm);
    cr_assert(fx.positionOf(id) == Position(0, 9));
}

Test(storm, start_broadcast_format)
{
    Storm storm(Position(5, 5), 3, Orientation::EAST, 2, 10);
    const std::string line = storm.startBroadcast();

    cr_assert_str_eq(line.c_str(), "evt_storm_start 5 5 3 2");
}

Test(storm, tick_broadcast_format)
{
    Storm storm(Position(5, 5), 3, Orientation::EAST, 2, 10);
    const std::string line = storm.tickBroadcast();

    cr_assert_str_eq(line.c_str(), "evt_storm_tick 5 5 3 2");
}

Test(storm, end_broadcast_format)
{
    Storm storm(Position(5, 5), 3, Orientation::EAST, 2, 10);
    const std::string line = storm.endBroadcast();

    cr_assert_str_eq(line.c_str(), "evt_storm_end");
}
