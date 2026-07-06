/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Biome-weighted resource spawning tests (ZAP-52, bonus)
*/

#include <criterion/criterion.h>

#include "server/game/Biome.hpp"
#include "server/game/Position.hpp"
#include "server/game/ResourceSpawner.hpp"
#include "server/game/World.hpp"

using zappy::server::game::Biome;
using zappy::server::game::Position;
using zappy::server::game::ResourceSpawner;
using zappy::server::game::ResourceType;
using zappy::server::game::World;

namespace {

int sumResource(const World &world, ResourceType type, int xStart, int xEnd)
{
    int total = 0;

    for (int x = xStart; x < xEnd; ++x)
        total += world.tileAt(Position(x, 0)).resource(type);
    return total;
}

void paintMountainHalf(World &world)
{
    for (int x = world.width() / 2; x < world.width(); ++x)
        world.setTileBiome(Position(x, 0), Biome::MOUNTAIN);
}

} // namespace

Test(resource_spawner_biomes, uniform_when_no_biome_variation)
{
    World world(10, 10, {"team"}, 8);
    ResourceSpawner spawner(123);

    spawner.spawnInitial(world);
    int total = 0;
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x)
            total += world.tileAt(Position(x, y)).resource(ResourceType::FOOD);
    cr_assert_eq(total,
        ResourceSpawner::targetCountFor(ResourceType::FOOD, 10, 10));
}

Test(resource_spawner_biomes, food_never_spawns_on_mountain)
{
    World world(100, 1, {"team"}, 8);
    ResourceSpawner spawner(123);

    paintMountainHalf(world);
    spawner.spawnInitial(world);
    cr_assert_eq(sumResource(world, ResourceType::FOOD, 50, 100), 0);
}

Test(resource_spawner_biomes, linemate_favors_mountain)
{
    World world(100, 1, {"team"}, 8);
    ResourceSpawner spawner(123);

    paintMountainHalf(world);
    spawner.spawnInitial(world);
    const int plain = sumResource(world, ResourceType::LINEMATE, 0, 50);
    const int mountain = sumResource(world, ResourceType::LINEMATE, 50, 100);
    cr_assert_gt(mountain, plain);
}
