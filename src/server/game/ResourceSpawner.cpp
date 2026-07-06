/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Initial resource seeding into a World
*/

#include "server/game/ResourceSpawner.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "server/game/Position.hpp"
#include "server/game/ResourceDensity.hpp"
#include "server/game/World.hpp"

zappy::server::game::ResourceSpawner::ResourceSpawner(std::uint64_t seed)
    : _rng(seed)
{}

void zappy::server::game::ResourceSpawner::spawnInitial(World &world)
{
    const bool biomes = hasBiomeVariation(world);

    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
        const ResourceType type = static_cast<ResourceType>(i);
        const int target = biomes
            ? targetCountForWithBiomes(world, type)
            : targetCountFor(type, world.width(), world.height());
        spawnUnits(world, type, target, biomes);
    }
}

void zappy::server::game::ResourceSpawner::refillMissing(World &world)
{
    const bool biomes = hasBiomeVariation(world);

    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
        const ResourceType type = static_cast<ResourceType>(i);
        const int target = biomes
            ? targetCountForWithBiomes(world, type)
            : targetCountFor(type, world.width(), world.height());
        const int missing = target - countResourceOnMap(world, type);
        if (missing > 0)
            spawnUnits(world, type, missing, biomes);
    }
}

int zappy::server::game::ResourceSpawner::countResourceOnMap(
    const World &world, ResourceType type) const
{
    int total = 0;
    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            total += world.tileAt(Position(x, y)).resource(type);
    return total;
}

int zappy::server::game::ResourceSpawner::targetCountFor(
    ResourceType type, int width, int height) noexcept
{
    const double area = static_cast<double>(width) * height;
    const double raw = std::round(area * ResourceDensity::densityFor(type));
    return std::max(1, static_cast<int>(raw));
}

void zappy::server::game::ResourceSpawner::spawnOne(
    World &world, ResourceType type, int count)
{
    std::uniform_int_distribution<int> xDist(0, world.width() - 1);
    std::uniform_int_distribution<int> yDist(0, world.height() - 1);
    for (int i = 0; i < count; ++i)
        world.addTileResource(Position(xDist(_rng), yDist(_rng)), type, 1);
}

void zappy::server::game::ResourceSpawner::spawnUnits(
    World &world, ResourceType type, int count, bool biomes)
{
    if (biomes)
        spawnOneWithBiomes(world, type, count);
    else
        spawnOne(world, type, count);
}

bool zappy::server::game::ResourceSpawner::hasBiomeVariation(
    const World &world) const
{
    const int total = world.width() * world.height();
    bool found = false;

    for (int i = 0; i < total && !found; ++i)
        found = world.tileAt(Position(i % world.width(), i / world.width()))
            .biome() != Biome::PLAIN;
    return found;
}

std::vector<double> zappy::server::game::ResourceSpawner::tileWeights(
    const World &world, std::size_t index) const
{
    std::vector<double> weights;

    weights.reserve(static_cast<std::size_t>(world.width()) * world.height());
    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            weights.push_back(biomeInfoFor(
                world.tileAt(Position(x, y)).biome())
                .densityMultipliers[index]);
    return weights;
}

int zappy::server::game::ResourceSpawner::targetCountForWithBiomes(
    const World &world, ResourceType type) const
{
    const std::size_t index = static_cast<std::size_t>(type);
    const double base = ResourceDensity::densityFor(type);
    double weighted = 0.0;

    for (double multiplier : tileWeights(world, index))
        weighted += base * multiplier;
    return weighted > 0.0
        ? std::max(1, static_cast<int>(std::round(weighted)))
        : 0;
}

void zappy::server::game::ResourceSpawner::spawnOneWithBiomes(
    World &world, ResourceType type, int count)
{
    const int width = world.width();
    const std::size_t index = static_cast<std::size_t>(type);
    std::vector<double> weights = tileWeights(world, index);
    std::discrete_distribution<int> dist(weights.begin(), weights.end());

    for (int i = 0; i < count; ++i) {
        const int tile = dist(_rng);
        world.addTileResource(Position(tile % width, tile / width), type, 1);
    }
}
