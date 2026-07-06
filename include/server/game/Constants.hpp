/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Game-layer enums and shared aliases
*/

#ifndef SERVER_GAME_CONSTANTS_HPP_
    #define SERVER_GAME_CONSTANTS_HPP_

    #include <array>
    #include <cstddef>
    #include <string_view>

namespace zappy::server::game {

/**
 * @brief Cardinal facing of a player, matching the GUI protocol values.
 */
enum class Orientation { NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4 };

/**
 * @brief The seven Trantorian resources, ordered as inventory indices.
 */
enum class ResourceType {
    FOOD = 0,
    LINEMATE = 1,
    DERAUMERE = 2,
    SIBUR = 3,
    MENDIANE = 4,
    PHIRAS = 5,
    THYSTAME = 6
};

/**
 * @brief Lifecycle state of a player.
 */
enum class PlayerState { ALIVE, INCANTING, DEAD };

/**
 * @brief Lifecycle state of an egg.
 */
enum class EggState { WAITING, HATCHED };

constexpr std::size_t RESOURCE_COUNT = 7;

/**
 * @brief Resource names indexed by static_cast<size_t>(ResourceType).
 */
inline constexpr std::array<std::string_view, RESOURCE_COUNT> RESOURCE_NAMES = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"
};

constexpr int FOOD_CONSUMPTION_INTERVAL_TIME_UNITS = 126;

constexpr int FOOD_FLOOD_MULTIPLIER = 3;

constexpr int INITIAL_FOOD = 10;

using Inventory = std::array<int, RESOURCE_COUNT>;

} // namespace zappy::server::game

#endif /* !SERVER_GAME_CONSTANTS_HPP_ */
