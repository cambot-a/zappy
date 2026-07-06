/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the Meteor natural event (ZAP-51, bonus)
*/

#include <criterion/criterion.h>
#include <cstddef>
#include <string>
#include <vector>

#include "server/event/Meteor.hpp"
#include "server/game/Constants.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::event::Meteor;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;

namespace {

struct MeteorFixture {
    World world{10, 10, {"team"}, 8};
    std::vector<int> killed;

    Meteor make(Position center, int radius)
    {
        return Meteor(center, radius,
            [this](int id) { killed.push_back(id); });
    }

    void fillTile(Position pos)
    {
        for (std::size_t i = 0; i < zappy::server::game::RESOURCE_COUNT; i++)
            world.setTileResource(pos, static_cast<ResourceType>(i), 4);
    }

    bool tileEmpty(Position pos)
    {
        for (std::size_t i = 0; i < zappy::server::game::RESOURCE_COUNT; i++)
            if (world.tileAt(pos).resource(static_cast<ResourceType>(i)) != 0)
                return false;
        return true;
    }
};

class TileChangeCounter : public WorldObserverAdapter {
public:
    void onTileChanged(Position) override { changes++; }
    int changes = 0;
};

} // namespace

Test(meteor, one_shot_expires_immediately)
{
    MeteorFixture fx;
    Meteor meteor = fx.make(Position(5, 5), 1);

    cr_assert_not(meteor.applyTick(fx.world));
}

Test(meteor, destroys_resources_in_zone)
{
    MeteorFixture fx;
    fx.fillTile(Position(5, 5));
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert(fx.tileEmpty(Position(5, 5)));
}

Test(meteor, keeps_resources_outside_zone)
{
    MeteorFixture fx;
    fx.fillTile(Position(5, 5));
    fx.fillTile(Position(8, 8));
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert(fx.tileEmpty(Position(5, 5)));
    cr_assert_not(fx.tileEmpty(Position(8, 8)));
}

Test(meteor, impact_zone_wraps_on_torus)
{
    MeteorFixture fx;
    fx.fillTile(Position(9, 9));
    fx.fillTile(Position(0, 0));
    fx.fillTile(Position(8, 9));
    fx.fillTile(Position(9, 0));
    Meteor meteor = fx.make(Position(9, 9), 2);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert(fx.tileEmpty(Position(9, 9)));
    cr_assert(fx.tileEmpty(Position(0, 0)));
    cr_assert(fx.tileEmpty(Position(8, 9)));
    cr_assert(fx.tileEmpty(Position(9, 0)));
}

Test(meteor, kills_player_in_zone)
{
    MeteorFixture fx;
    const int id = fx.world.addPlayer("team", Position(5, 5),
        Orientation::NORTH);
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert_eq(fx.killed.size(), 1U);
    cr_assert_eq(fx.killed.front(), id);
}

Test(meteor, spares_player_outside_zone)
{
    MeteorFixture fx;
    fx.world.addPlayer("team", Position(8, 8), Orientation::NORTH);
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert_eq(fx.killed.size(), 0U);
}

Test(meteor, kills_every_player_in_zone)
{
    MeteorFixture fx;
    fx.world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    fx.world.addPlayer("team", Position(4, 5), Orientation::NORTH);
    fx.world.addPlayer("team", Position(6, 5), Orientation::NORTH);
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert_eq(fx.killed.size(), 3U);
}

Test(meteor, does_not_rekill_dead_player)
{
    MeteorFixture fx;
    const int id = fx.world.addPlayer("team", Position(5, 5),
        Orientation::NORTH);
    fx.world.killPlayer(id);
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert_eq(fx.killed.size(), 0U);
}

Test(meteor, fires_tile_changed_observer)
{
    MeteorFixture fx;
    TileChangeCounter obs;
    fx.world.addObserver(obs);
    Meteor meteor = fx.make(Position(5, 5), 1);

    static_cast<void>(meteor.applyTick(fx.world));
    cr_assert_gt(obs.changes, 0);
}

Test(meteor, start_broadcast_format)
{
    MeteorFixture fx;
    Meteor meteor = fx.make(Position(5, 5), 2);
    const std::string line = meteor.startBroadcast();

    cr_assert_str_eq(line.c_str(), "evt_meteor_impact 5 5 2");
}
