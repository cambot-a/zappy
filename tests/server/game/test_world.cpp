/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for World, including observer notifications
*/

#include <criterion/criterion.h>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/game/WorldError.hpp"

using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldError;
using zappy::server::game::WorldObserverAdapter;

/* helpers */

static World makeWorld(int slots = 2)
{
    return World(10, 10, {"alpha", "beta"}, slots);
}

/**
 * @brief Records the ordered sequence of world events as tagged strings.
 */
class MockObserver : public WorldObserverAdapter {
public:
    std::vector<std::string> events;

    void onTileChanged(Position) override { events.push_back("tile"); }
    void onPlayerAdded(int id) override { tag("padd", id); }
    void onPlayerMoved(int id, Position, Position) override { tag("pmove", id); }
    void onPlayerRotated(int id) override { tag("prot", id); }
    void onPlayerLevelChanged(int id) override { tag("plvl", id); }
    void onPlayerInventoryChanged(int id) override { tag("pinv", id); }
    void onPlayerStateChanged(int id) override { tag("pstate", id); }
    void onPlayerRemoved(int id) override { tag("prem", id); }
    void onEggAdded(int id) override { tag("eadd", id); }
    void onEggHatched(int id) override { tag("ehatch", id); }
    void onEggRemoved(int id) override { tag("erem", id); }
    void onTeamSlotsChanged(const std::string &t) override
    {
        events.push_back("slots:" + t);
    }

private:
    void tag(const std::string &kind, int id)
    {
        events.push_back(kind + ":" + std::to_string(id));
    }
};

/* construction */

Test(world, dimensions)
{
    const World w = makeWorld();
    cr_assert_eq(w.width(), 10);
    cr_assert_eq(w.height(), 10);
}

Test(world, tiles_start_empty)
{
    const World w = makeWorld();
    cr_assert_eq(w.tileAt(Position(0, 0)).playerIds().size(), 0U);
    cr_assert_eq(w.tileAt(Position(9, 9)).resource(ResourceType::FOOD), 0);
}

Test(world, teams_created_with_slots)
{
    const World w = makeWorld(3);
    cr_assert_eq(w.teams().size(), 2U);
    cr_assert(w.hasTeam("alpha"));
    cr_assert(w.hasTeam("beta"));
    cr_assert_eq(w.team("alpha").slotsTotal(), 3);
}

Test(world, unknown_team_throws)
{
    const World w = makeWorld();
    cr_assert_throw((void)w.team("ghost"), WorldError);
}

/* geometry */

Test(world, tile_at_raw_normalizes)
{
    World w = makeWorld();
    w.tileAtRaw(Position(-1, -1)).addPlayer(42);
    cr_assert_eq(w.tileAt(Position(9, 9)).playerIds().size(), 1U);
}

/* players */

Test(world, add_player_happy_path)
{
    World w = makeWorld(2);
    const int id = w.addPlayer("alpha", Position(1, 1), Orientation::NORTH);
    cr_assert(w.hasPlayer(id));
    cr_assert_eq(w.player(id).team().c_str()[0], 'a');
    cr_assert_eq(w.tileAt(Position(1, 1)).playerIds()[0], id);
}

Test(world, add_player_unknown_team_throws)
{
    World w = makeWorld();
    cr_assert_throw(w.addPlayer("ghost", Position(0, 0), Orientation::NORTH),
        WorldError);
}

Test(world, remove_player_clears_tile)
{
    World w = makeWorld(2);
    const int id = w.addPlayer("alpha", Position(1, 1), Orientation::NORTH);
    w.removePlayer(id);
    cr_assert_not(w.hasPlayer(id));
    cr_assert_eq(w.tileAt(Position(1, 1)).playerIds().size(), 0U);
}

Test(world, move_player_updates_both_tiles)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(1, 1), Orientation::NORTH);
    w.movePlayer(id, Position(2, 2));
    cr_assert_eq(w.tileAt(Position(1, 1)).playerIds().size(), 0U);
    cr_assert_eq(w.tileAt(Position(2, 2)).playerIds()[0], id);
    cr_assert(w.player(id).position() == Position(2, 2));
}

Test(world, rotate_player)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(0, 0), Orientation::NORTH);
    w.rotatePlayer(id, Orientation::WEST);
    cr_assert_eq(static_cast<int>(w.player(id).orientation()),
        static_cast<int>(Orientation::WEST));
}

/* resources */

Test(world, take_resource_success)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(3, 3), Orientation::NORTH);
    w.setTileResource(Position(3, 3), ResourceType::LINEMATE, 1);
    cr_assert(w.takeResourceFromTile(id, ResourceType::LINEMATE));
    cr_assert_eq(w.player(id).resource(ResourceType::LINEMATE), 1);
    cr_assert_eq(w.tileAt(Position(3, 3)).resource(ResourceType::LINEMATE), 0);
}

Test(world, take_resource_absent_fails)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(3, 3), Orientation::NORTH);
    cr_assert_not(w.takeResourceFromTile(id, ResourceType::THYSTAME));
}

Test(world, drop_resource_success)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(4, 4), Orientation::NORTH);
    cr_assert(w.dropResourceOnTile(id, ResourceType::FOOD));
    cr_assert_eq(w.player(id).resource(ResourceType::FOOD), 9);
    cr_assert_eq(w.tileAt(Position(4, 4)).resource(ResourceType::FOOD), 1);
}

Test(world, drop_resource_absent_fails)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(4, 4), Orientation::NORTH);
    cr_assert_not(w.dropResourceOnTile(id, ResourceType::SIBUR));
}

/* kill */

Test(world, kill_player)
{
    World w = makeWorld(2);
    const int id = w.addPlayer("alpha", Position(5, 5), Orientation::NORTH);
    w.killPlayer(id);
    cr_assert_eq(static_cast<int>(w.player(id).state()),
        static_cast<int>(zappy::server::game::PlayerState::DEAD));
    cr_assert_eq(w.tileAt(Position(5, 5)).playerIds().size(), 0U);
}

/* eggs */

Test(world, add_and_remove_egg)
{
    World w = makeWorld();
    const int id = w.addEgg("alpha", Position(6, 6));
    cr_assert(w.hasEgg(id));
    cr_assert_eq(w.tileAt(Position(6, 6)).eggIds()[0], id);
    w.removeEgg(id);
    cr_assert_not(w.hasEgg(id));
    cr_assert_eq(w.tileAt(Position(6, 6)).eggIds().size(), 0U);
}

Test(world, hatch_egg_keeps_it)
{
    World w = makeWorld();
    const int id = w.addEgg("alpha", Position(6, 6));
    w.hatchEgg(id);
    cr_assert(w.hasEgg(id));
    cr_assert_eq(static_cast<int>(w.egg(id).state()),
        static_cast<int>(zappy::server::game::EggState::HATCHED));
}

Test(world, waiting_egg_count_counts_team_waiting)
{
    World w = makeWorld();
    w.addEgg("alpha", Position(1, 1));
    w.addEgg("alpha", Position(2, 2));
    w.addEgg("beta", Position(3, 3));
    cr_assert_eq(w.waitingEggCount("alpha"), 2);
    cr_assert_eq(w.waitingEggCount("beta"), 1);
}

Test(world, waiting_egg_count_ignores_hatched)
{
    World w = makeWorld();
    const int id = w.addEgg("alpha", Position(1, 1));
    w.hatchEgg(id);
    cr_assert_eq(w.waitingEggCount("alpha"), 0);
}

Test(world, waiting_egg_count_unknown_team_is_zero)
{
    World w = makeWorld();
    w.addEgg("alpha", Position(1, 1));
    cr_assert_eq(w.waitingEggCount("ghost"), 0);
}

Test(world, pick_random_waiting_egg_none_returns_nullopt)
{
    const World w = makeWorld();
    std::mt19937_64 rng(42);
    cr_assert_not(w.pickRandomWaitingEgg("alpha", rng).has_value());
}

Test(world, pick_random_waiting_egg_seeded_is_deterministic)
{
    World w = makeWorld();
    w.addEgg("alpha", Position(1, 1));
    w.addEgg("alpha", Position(2, 2));
    w.addEgg("alpha", Position(3, 3));
    std::mt19937_64 rngA(123);
    std::mt19937_64 rngB(123);
    const std::optional<int> a = w.pickRandomWaitingEgg("alpha", rngA);
    const std::optional<int> b = w.pickRandomWaitingEgg("alpha", rngB);
    cr_assert(a.has_value());
    cr_assert_eq(*a, *b);
}

Test(world, pick_random_waiting_egg_only_matching_team)
{
    World w = makeWorld();
    const int egg = w.addEgg("alpha", Position(4, 4));
    w.addEgg("beta", Position(5, 5));
    std::mt19937_64 rng(7);
    const std::optional<int> picked = w.pickRandomWaitingEgg("alpha", rng);
    cr_assert(picked.has_value());
    cr_assert_eq(*picked, egg);
}

Test(world, grow_team_increments_slots_and_notifies)
{
    World w = makeWorld(2);
    MockObserver obs;
    w.addObserver(obs);
    w.growTeam("alpha");
    cr_assert_eq(w.team("alpha").slotsTotal(), 3);
    cr_assert_eq(obs.events.size(), 1U);
    cr_assert_str_eq(obs.events[0].c_str(), "slots:alpha");
}

Test(world, grow_team_unknown_throws)
{
    World w = makeWorld();
    cr_assert_throw(w.growTeam("ghost"), WorldError);
}

/* observer */

Test(world, observer_receives_events_in_order)
{
    World w = makeWorld(2);
    MockObserver obs;
    w.addObserver(obs);
    const int id = w.addPlayer("alpha", Position(1, 1), Orientation::NORTH);
    w.movePlayer(id, Position(2, 2));
    w.rotatePlayer(id, Orientation::EAST);
    cr_assert_eq(obs.events.size(), 3U);
    cr_assert_str_eq(obs.events[0].c_str(), "padd:1");
    cr_assert_str_eq(obs.events[1].c_str(), "pmove:1");
    cr_assert_str_eq(obs.events[2].c_str(), "prot:1");
}

Test(world, observer_take_resource_fires_tile_and_inventory)
{
    World w = makeWorld();
    const int id = w.addPlayer("alpha", Position(3, 3), Orientation::NORTH);
    w.setTileResource(Position(3, 3), ResourceType::SIBUR, 1);
    MockObserver obs;
    w.addObserver(obs);
    cr_assert(w.takeResourceFromTile(id, ResourceType::SIBUR));
    cr_assert_eq(obs.events.size(), 2U);
    cr_assert_str_eq(obs.events[0].c_str(), "tile");
    cr_assert_str_eq(obs.events[1].c_str(), "pinv:1");
}

Test(world, removed_observer_stops_receiving)
{
    World w = makeWorld();
    MockObserver obs;
    w.addObserver(obs);
    const int id = w.addPlayer("alpha", Position(1, 1), Orientation::NORTH);
    const std::size_t before = obs.events.size();
    w.removeObserver(obs);
    w.rotatePlayer(id, Orientation::SOUTH);
    cr_assert_eq(obs.events.size(), before);
}
