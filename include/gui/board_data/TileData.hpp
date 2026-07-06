/*
** EPITECH PROJECT, 2026
** TileData.hpp
** File description:
** TileData
*/

#ifndef TILEDATA_HPP_
    #define TILEDATA_HPP_
    #include <array>
    #include <cstddef>
    #include <string_view>

namespace zappy::gui::board_data {

constexpr std::size_t RESOURCE_COUNT = 7;
constexpr std::size_t BIOME_COUNT = 6;

using Resources = std::array<int, RESOURCE_COUNT>;

enum class Orientation { NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4 };

enum class Biome { PLAIN, VALLEY, PLATEAU, MOUNTAIN, PEAK, SNOW_PLAIN };

[[nodiscard]] Biome biome_from_name(std::string_view name) noexcept;

struct TileData {
    int x = 0;
    int y = 0;
    Resources res{};
    Biome biome = Biome::PLAIN;
    bool flooded = false;
};

} // namespace zappy::gui::board_data

#endif /* TILEDATA_HPP_ */
