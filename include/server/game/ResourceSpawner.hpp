/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Initial resource seeding into a World
*/

#ifndef SERVER_GAME_RESOURCESPAWNER_HPP_
    #define SERVER_GAME_RESOURCESPAWNER_HPP_

    #include <cstdint>
    #include <random>
    #include <vector>

    #include "server/game/Biome.hpp"
    #include "server/game/Constants.hpp"

namespace zappy::server::game {

class World;

/**
 * @brief Seeds the map with the initial resource stock at server boot.
 */
class ResourceSpawner {
public:
    /**
     * @brief Build a spawner with a seeded random engine.
     *
     * @param seed value feeding the random engine
     */
    explicit ResourceSpawner(std::uint64_t seed = std::random_device{}());

    /**
     * @brief Drop the target amount of every resource over the grid.
     *
     * @param world world to seed in place
     */
    void spawnInitial(World &world);

    /**
     * @brief Top up every resource short of its target, never removing surplus.
     *
     * @param world world to refill in place
     */
    void refillMissing(World &world);

    /**
     * @brief Target unit count for a resource on a given map size.
     *
     * @param type resource to size
     * @param width map width in tiles
     * @param height map height in tiles
     * @return int the clamped, rounded target count
     */
    [[nodiscard]] static int targetCountFor(ResourceType type,
        int width, int height) noexcept;

private:
    /**
     * @brief Scatter @p count units of @p type over random tiles.
     *
     * @param world world to seed in place
     * @param type resource being dropped
     * @param count number of units to drop
     */
    void spawnOne(World &world, ResourceType type, int count);

    /**
     * @brief Scatter @p count units of @p type weighted by biome multipliers.
     *
     * @param world world to seed in place
     * @param type resource being dropped
     * @param count number of units to drop
     */
    void spawnOneWithBiomes(World &world, ResourceType type, int count);

    /**
     * @brief Biome-weighted target count for @p type across the whole map.
     *
     * @param world world supplying the per-tile biomes
     * @param type resource to size
     * @return int the clamped, rounded target count
     */
    [[nodiscard]] int targetCountForWithBiomes(
        const World &world, ResourceType type) const;

    /**
     * @brief Whether any tile carries a non-PLAIN biome.
     *
     * @param world world to scan
     * @return bool true if biome variation is present
     */
    [[nodiscard]] bool hasBiomeVariation(const World &world) const;

    /**
     * @brief Drop @p count units of @p type using the matching spawn strategy.
     *
     * @param world world to seed in place
     * @param type resource being dropped
     * @param count number of units to drop
     * @param biomes whether to weight placement by biome multipliers
     */
    void spawnUnits(World &world, ResourceType type, int count, bool biomes);

    /**
     * @brief Per-tile spawn weight of resource @p index, in row-major order.
     *
     * @param world world supplying the per-tile biomes
     * @param index inventory index of the resource
     * @return std::vector<double> one weight per tile
     */
    [[nodiscard]] std::vector<double> tileWeights(
        const World &world, std::size_t index) const;

    /**
     * @brief Total units of @p type currently present across all tiles.
     *
     * @param world world to scan
     * @param type resource being counted
     * @return int the summed quantity
     */
    [[nodiscard]] int countResourceOnMap(
        const World &world, ResourceType type) const;

    std::mt19937_64 _rng;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_RESOURCESPAWNER_HPP_ */
