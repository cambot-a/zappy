/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Boot-time biome blob generation over a World (ZAP-52, bonus)
*/

#ifndef SERVER_GAME_BIOMEMAPGENERATOR_HPP_
    #define SERVER_GAME_BIOMEMAPGENERATOR_HPP_

    #include <cstdint>
    #include <random>

    #include "server/game/Biome.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {

class World;

/**
 * @brief Paints biome blobs over the map at server boot.
 *
 * Generation is a handful of random bounding-box blobs of non-PLAIN biomes
 * scattered over the grid; uncovered tiles keep the default PLAIN biome and
 * overlapping blobs simply override one another. Seeding the engine makes a
 * run reproducible.
 */
class BiomeMapGenerator {
public:
    /**
     * @brief Build a generator with a seeded random engine.
     *
     * @param seed value feeding the random engine
     */
    explicit BiomeMapGenerator(
        std::uint64_t seed = std::random_device{}());

    /**
     * @brief Paint biome blobs over @p world in place.
     *
     * @param world world whose tiles receive biomes
     */
    void generate(World &world);

private:
    /**
     * @brief Scatter the random biome blobs over the grid.
     *
     * @param world world to paint
     */
    void placeBiomes(World &world);

    /**
     * @brief Pick one non-PLAIN biome uniformly at random.
     *
     * @return Biome the chosen biome
     */
    [[nodiscard]] Biome pickRandomBiome();

    /**
     * @brief Paint a square blob of @p biome around @p center.
     *
     * @param world world to paint
     * @param center blob centre on the map
     * @param biome biome to assign over the blob
     * @param radius blob half-side in tiles
     */
    void floodFillBiome(World &world, Position center, Biome biome,
        int radius);

    std::mt19937_64 _rng;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_BIOMEMAPGENERATOR_HPP_ */
