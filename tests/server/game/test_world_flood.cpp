/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for World::setTileFlooded and its observer notifications
*/

#include <criterion/criterion.h>

#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;

namespace {

class FloodObserver : public WorldObserverAdapter {
public:
    void onTileChanged(Position pos) override
    {
        tileChanged = true;
        lastChanged = pos;
    }

    void onTileFloodChanged(Position pos, bool flooded) override
    {
        floodChanged = true;
        lastFlood = pos;
        lastValue = flooded;
    }

    bool tileChanged = false;
    bool floodChanged = false;
    Position lastChanged{};
    Position lastFlood{};
    bool lastValue = false;
};

} // namespace

Test(world_flood, set_tile_flooded_fires_both_notifications)
{
    World world(10, 10, {"team"}, 8);
    FloodObserver obs;
    world.addObserver(obs);

    world.setTileFlooded(Position(3, 4), true);
    cr_assert(obs.tileChanged);
    cr_assert(obs.floodChanged);
    cr_assert(obs.lastChanged == Position(3, 4));
    cr_assert(obs.lastFlood == Position(3, 4));
    cr_assert(obs.lastValue);
}

Test(world_flood, set_tile_flooded_flips_tile_state)
{
    World world(10, 10, {"team"}, 8);

    world.setTileFlooded(Position(3, 4), true);
    cr_assert(world.tileAt(Position(3, 4)).isFlooded());
    world.setTileFlooded(Position(3, 4), false);
    cr_assert_not(world.tileAt(Position(3, 4)).isFlooded());
}
