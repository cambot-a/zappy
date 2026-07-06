/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Static elevation requirements per starting level
*/

#ifndef SERVER_GAME_ELEVATIONRULES_HPP_
    #define SERVER_GAME_ELEVATIONRULES_HPP_

    #include <array>
    #include <cstddef>

    #include "server/game/Constants.hpp"

namespace zappy::server::game {

/**
 * @brief Players and stones required to elevate from one level to the next.
 */
struct ElevationRule {
    int playersRequired;
    std::array<int, RESOURCE_COUNT> stonesRequired;
};

/**
 * @brief Read-only lookup of the elevation rule for a given starting level.
 */
class ElevationRules {
public:
    /**
     * @brief Highest reachable level; level 8 is terminal.
     *
     * @return int the maximum level
     */
    [[nodiscard]] static constexpr int maxLevel() noexcept { return 8; }

    /**
     * @brief Rule to elevate from @p level to @p level + 1.
     *
     * @param level starting level (1..7)
     * @return const ElevationRule& the matching rule, or zeros if out of range
     */
    [[nodiscard]] static constexpr const ElevationRule &forLevel(
        int level) noexcept
    {
        const bool inRange = level >= 1 && level <= 7;
        return inRange ? RULES[static_cast<std::size_t>(level - 1)] : ZERO_RULE;
    }

private:
    static constexpr ElevationRule ZERO_RULE{0, {0, 0, 0, 0, 0, 0, 0}};

    static constexpr std::array<ElevationRule, 7> RULES = {{
        {1, {0, 1, 0, 0, 0, 0, 0}},
        {2, {0, 1, 1, 1, 0, 0, 0}},
        {2, {0, 2, 0, 1, 0, 2, 0}},
        {4, {0, 1, 1, 2, 0, 1, 0}},
        {4, {0, 1, 2, 1, 3, 0, 0}},
        {6, {0, 1, 2, 3, 0, 1, 0}},
        {6, {0, 2, 2, 2, 2, 2, 1}}
    }};
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_ELEVATIONRULES_HPP_ */
