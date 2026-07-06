/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the initial resource spawner
*/

#include <criterion/criterion.h>
#include <algorithm>
#include <array>
#include <cstddef>

#include "server/game/Constants.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/ResourceDensity.hpp"
#include "server/game/ResourceSpawner.hpp"
#include "server/game/World.hpp"

using zappy::server::game::Position;
using zappy::server::game::ResourceSpawner;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::game::RESOURCE_COUNT;

static World makeWorld(int width = 10, int height = 10)
{
    return World(width, height, {"alpha", "beta"}, 2);
}

/**
 * @brief Sums all units of a resource type currently on the grid.
 */
static int totalOnMap(const World &world, ResourceType type)
{
    int total = 0;
    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            total += world.tileAt(Position(x, y)).resource(type);
    return total;
}

/**
 * @brief Counts onTileChanged notifications received from the world.
 */
class TileCounter : public WorldObserverAdapter {
public:
    int tileChanges = 0;

    void onTileChanged(Position) override { ++tileChanges; }
};

/* target count formula */

Test(resource_spawner, target_counts_on_10x10)
{
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::FOOD, 10, 10), 50);
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::LINEMATE, 10, 10), 30);
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::DERAUMERE, 10, 10), 15);
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::SIBUR, 10, 10), 10);
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::MENDIANE, 10, 10), 10);
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::PHIRAS, 10, 10), 8);
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::THYSTAME, 10, 10), 5);
}

Test(resource_spawner, target_count_floor_is_one_on_1x1)
{
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
        const ResourceType type = static_cast<ResourceType>(i);
        cr_assert_eq(ResourceSpawner::targetCountFor(type, 1, 1), 1);
    }
}

Test(resource_spawner, target_count_floor_is_one_on_0x0)
{
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
        const ResourceType type = static_cast<ResourceType>(i);
        cr_assert_eq(ResourceSpawner::targetCountFor(type, 0, 0), 1);
    }
}

Test(resource_spawner, target_count_rounds_thystame_on_7x7)
{
    cr_assert_eq(ResourceSpawner::targetCountFor(ResourceType::THYSTAME, 7, 7), 2);
}

/* spawn totals */

Test(resource_spawner, spawn_produces_exact_totals)
{
    World world = makeWorld();
    ResourceSpawner spawner(42);
    spawner.spawnInitial(world);
    const std::array<int, RESOURCE_COUNT> expected = {50, 30, 15, 10, 10, 8, 5};
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
        const ResourceType type = static_cast<ResourceType>(i);
        cr_assert_eq(totalOnMap(world, type), expected[i]);
        cr_assert_geq(totalOnMap(world, type), 1);
    }
}

/* observer events */

Test(resource_spawner, spawn_fires_one_event_per_unit)
{
    World world = makeWorld();
    TileCounter counter;
    world.addObserver(counter);
    ResourceSpawner spawner(42);
    spawner.spawnInitial(world);
    cr_assert_eq(counter.tileChanges, 128);
}

/* spread */

Test(resource_spawner, spread_no_tile_hoards_resources)
{
    World world = makeWorld();
    ResourceSpawner spawner(42);
    spawner.spawnInitial(world);
    int total = 0;
    int maxOnTile = 0;
    for (int y = 0; y < world.height(); ++y) {
        for (int x = 0; x < world.width(); ++x) {
            int onTile = 0;
            for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
                onTile += world.tileAt(Position(x, y))
                    .resource(static_cast<ResourceType>(i));
            total += onTile;
            maxOnTile = std::max(maxOnTile, onTile);
        }
    }
    cr_assert_lt(maxOnTile, total * 0.30);
}
