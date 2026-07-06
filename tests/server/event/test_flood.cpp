/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the Flood natural event (ZAP-50, bonus)
*/

#include <criterion/criterion.h>
#include <string>

#include "server/event/Flood.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::event::Flood;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;

namespace {

class FloodCounter : public WorldObserverAdapter {
public:
    void onTileFloodChanged(Position, bool flooded) override
    {
        if (flooded)
            onCount++;
        else
            offCount++;
    }

    int onCount = 0;
    int offCount = 0;
};

bool allFlooded(World &world, Position origin, int w, int h, bool expected)
{
    for (int dy = 0; dy < h; dy++)
        for (int dx = 0; dx < w; dx++) {
            const Position p = Position(origin.x() + dx, origin.y() + dy)
                .normalized(world.width(), world.height());
            if (world.tileAt(p).isFlooded() != expected)
                return false;
        }
    return true;
}

} // namespace

Test(flood, duration_five_stays_active_for_five_ticks)
{
    World world(10, 10, {"team"}, 8);
    Flood flood(Position(1, 1), 3, 3, 5);

    for (int i = 0; i < 5; i++)
        cr_assert(flood.applyTick(world));
    cr_assert_not(flood.applyTick(world));
}

Test(flood, first_tick_marks_whole_bbox)
{
    World world(10, 10, {"team"}, 8);
    Flood flood(Position(1, 1), 3, 3, 5);

    static_cast<void>(flood.applyTick(world));
    cr_assert(allFlooded(world, Position(1, 1), 3, 3, true));
}

Test(flood, on_end_unmarks_whole_bbox)
{
    World world(10, 10, {"team"}, 8);
    Flood flood(Position(1, 1), 3, 3, 5);

    static_cast<void>(flood.applyTick(world));
    flood.onEnd(world);
    cr_assert(allFlooded(world, Position(1, 1), 3, 3, false));
}

Test(flood, observer_fires_on_apply_and_cleanup)
{
    World world(10, 10, {"team"}, 8);
    FloodCounter obs;
    world.addObserver(obs);
    Flood flood(Position(1, 1), 3, 3, 5);

    static_cast<void>(flood.applyTick(world));
    cr_assert_eq(obs.onCount, 9);
    flood.onEnd(world);
    cr_assert_eq(obs.offCount, 9);
}

Test(flood, bbox_wraps_on_torus)
{
    World world(10, 10, {"team"}, 8);
    Flood flood(Position(9, 9), 2, 2, 5);

    static_cast<void>(flood.applyTick(world));
    cr_assert(world.tileAt(Position(9, 9)).isFlooded());
    cr_assert(world.tileAt(Position(0, 9)).isFlooded());
    cr_assert(world.tileAt(Position(9, 0)).isFlooded());
    cr_assert(world.tileAt(Position(0, 0)).isFlooded());
}

Test(flood, on_end_before_start_clears_nothing)
{
    World world(10, 10, {"team"}, 8);
    FloodCounter obs;
    world.addObserver(obs);
    Flood flood(Position(1, 1), 3, 3, 5);

    flood.onEnd(world);
    cr_assert_eq(obs.offCount, 0);
}

Test(flood, start_broadcast_format)
{
    Flood flood(Position(1, 2), 3, 4, 8);
    const std::string line = flood.startBroadcast();

    cr_assert_str_eq(line.c_str(), "evt_flood_start 1 2 3 4");
}

Test(flood, tick_broadcast_is_empty)
{
    Flood flood(Position(1, 2), 3, 4, 8);
    const std::string line = flood.tickBroadcast();

    cr_assert_str_eq(line.c_str(), "");
}

Test(flood, end_broadcast_format)
{
    Flood flood(Position(1, 2), 3, 4, 8);
    const std::string line = flood.endBroadcast();

    cr_assert_str_eq(line.c_str(), "evt_flood_end");
}
