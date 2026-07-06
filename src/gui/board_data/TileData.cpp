/*
** EPITECH PROJECT, 2026
** TileData.cpp
** File description:
** TileData
*/

#include "gui/board_data/TileData.hpp"

#include <array>
#include <utility>

namespace zappy::gui::board_data {

namespace {

constexpr std::array<std::pair<std::string_view, Biome>, BIOME_COUNT> NAMES = {{
    {"plain", Biome::PLAIN}, {"valley", Biome::VALLEY},
    {"plateau", Biome::PLATEAU}, {"mountain", Biome::MOUNTAIN},
    {"peak", Biome::PEAK}, {"snow_plain", Biome::SNOW_PLAIN}
}};

} // namespace

Biome biome_from_name(std::string_view name) noexcept
{
    for (const auto &entry : NAMES)
        if (entry.first == name)
            return entry.second;
    return Biome::PLAIN;
}

} // namespace zappy::gui::board_data
