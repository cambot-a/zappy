/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the initial egg spawner
*/

#include <criterion/criterion.h>
#include <vector>

#include "server/game/Constants.hpp"
#include "server/game/EggSpawner.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::game::EggSpawner;
using zappy::server::game::EggState;
using zappy::server::game::Position;
using zappy::server::game::World;

static World makeWorld(int slots = 3)
{
    return World(10, 10, {"alpha", "beta"}, slots);
}

/**
 * @brief Collect every egg id currently laid on the grid.
 */
static std::vector<int> allEggIds(const World &world)
{
    std::vector<int> ids;
    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            for (const int id : world.tileAt(Position(x, y)).eggIds())
                ids.push_back(id);
    return ids;
}

Test(egg_spawner, spawns_slots_total_per_team)
{
    World w = makeWorld(3);
    EggSpawner spawner(42);
    spawner.spawnInitial(w);
    cr_assert_eq(w.waitingEggCount("alpha"), 3);
    cr_assert_eq(w.waitingEggCount("beta"), 3);
    cr_assert_eq(allEggIds(w).size(), 6U);
}

Test(egg_spawner, all_spawned_eggs_are_waiting)
{
    World w = makeWorld(2);
    EggSpawner spawner(7);
    spawner.spawnInitial(w);
    for (const int id : allEggIds(w))
        cr_assert_eq(static_cast<int>(w.egg(id).state()),
            static_cast<int>(EggState::WAITING));
}

Test(egg_spawner, same_seed_is_deterministic)
{
    World a = makeWorld(2);
    World b = makeWorld(2);
    EggSpawner(99).spawnInitial(a);
    EggSpawner(99).spawnInitial(b);
    for (int id = 1; id <= 4; ++id) {
        cr_assert(a.egg(id).position() == b.egg(id).position());
        cr_assert_str_eq(a.egg(id).team().c_str(), b.egg(id).team().c_str());
    }
}

Test(egg_spawner, eggs_spread_over_the_map)
{
    World w = makeWorld(5);
    EggSpawner spawner(2024);
    spawner.spawnInitial(w);
    const std::vector<int> ids = allEggIds(w);
    bool differs = false;
    for (const int id : ids)
        if (!(w.egg(id).position() == w.egg(ids.front()).position()))
            differs = true;
    cr_assert(differs);
}
