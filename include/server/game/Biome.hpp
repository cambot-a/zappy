/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Biome types and their per-resource density multipliers (ZAP-52, bonus)
*/

#ifndef SERVER_GAME_BIOME_HPP_
    #define SERVER_GAME_BIOME_HPP_

    #include <array>
    #include <cstddef>
    #include <string_view>

    #include "server/game/Constants.hpp"

namespace zappy::server::game {

/**
 * @brief Terrain type of a tile, shaping resource density and traversability.
 */
enum class Biome {
    PLAIN,
    VALLEY,
    PLATEAU,
    MOUNTAIN,
    PEAK,
    SNOW_PLAIN
};

constexpr std::size_t BIOME_COUNT = 6;

/**
 * @brief Static description of a biome: name, traversability and multipliers.
 */
struct BiomeInfo {
    std::string_view name;
    bool traversable;
    std::array<double, RESOURCE_COUNT> densityMultipliers;
};

/**
 * @brief Biome descriptions indexed by static_cast<size_t>(Biome).
 */
inline constexpr std::array<BiomeInfo, BIOME_COUNT> BIOME_INFOS = {{
    {"plain", true, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}},
    {"valley", true, {2.0, 0.5, 1.0, 1.5, 1.0, 0.5, 0.5}},
    {"plateau", true, {1.0, 1.5, 1.5, 1.0, 1.0, 1.0, 1.0}},
    {"mountain", false, {0.0, 2.0, 1.5, 1.0, 0.5, 1.5, 1.5}},
    {"peak", false, {0.0, 1.0, 1.0, 0.5, 0.5, 2.0, 3.0}},
    {"snow_plain", true, {0.5, 1.0, 1.0, 0.5, 1.5, 1.0, 1.0}}
}};

/**
 * @brief Static description of @p biome.
 *
 * @param biome the biome to describe
 * @return const BiomeInfo& its name, traversability and multipliers
 */
[[nodiscard]] constexpr const BiomeInfo &biomeInfoFor(Biome biome) noexcept
{
    return BIOME_INFOS[static_cast<std::size_t>(biome)];
}

} // namespace zappy::server::game

#endif /* !SERVER_GAME_BIOME_HPP_ */
