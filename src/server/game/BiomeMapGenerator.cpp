/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Boot-time biome blob generation over a World
*/

#include "server/game/BiomeMapGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "server/game/World.hpp"

zappy::server::game::BiomeMapGenerator::BiomeMapGenerator(std::uint64_t seed)
    : _rng(seed)
{
}

/**
 * @brief Paint biome blobs over @p world in place.
 *
 * @param world world whose tiles receive biomes
 */
void zappy::server::game::BiomeMapGenerator::generate(World &world)
{
    placeBiomes(world);
}

/**
 * @brief Scatter the random biome blobs over the grid.
 *
 * @param world world to paint
 */
void zappy::server::game::BiomeMapGenerator::placeBiomes(World &world)
{
    const int area = world.width() * world.height();
    const int blobs = std::max(1, static_cast<int>(std::sqrt(area)) / 2);
    const int maxRadius =
        std::max(1, std::max(world.width(), world.height()) / 4);
    std::uniform_int_distribution<int> xDist(0, world.width() - 1);
    std::uniform_int_distribution<int> yDist(0, world.height() - 1);
    std::uniform_int_distribution<int> rDist(1, maxRadius);

    for (int i = 0; i < blobs; i++)
        floodFillBiome(world, Position(xDist(_rng), yDist(_rng)),
            pickRandomBiome(), rDist(_rng));
}

/**
 * @brief Pick one non-PLAIN biome uniformly at random.
 *
 * @return Biome the chosen biome
 */
zappy::server::game::Biome
zappy::server::game::BiomeMapGenerator::pickRandomBiome()
{
    std::uniform_int_distribution<std::size_t> dist(1, BIOME_COUNT - 1);

    return static_cast<Biome>(dist(_rng));
}

/**
 * @brief Paint a square blob of @p biome around @p center.
 *
 * @param world world to paint
 * @param center blob centre on the map
 * @param biome biome to assign over the blob
 * @param radius blob half-side in tiles
 */
void zappy::server::game::BiomeMapGenerator::floodFillBiome(World &world,
    Position center, Biome biome, int radius)
{
    const int side = 2 * radius + 1;

    for (int i = 0; i < side * side; i++)
        world.setTileBiome(Position(center.x() + i % side - radius,
            center.y() + i / side - radius)
            .normalized(world.width(), world.height()), biome);
}
