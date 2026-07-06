/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Canonical spawn densities for the seven resources
*/

#ifndef SERVER_GAME_RESOURCEDENSITY_HPP_
    #define SERVER_GAME_RESOURCEDENSITY_HPP_

    #include <array>
    #include <cstddef>

    #include "server/game/Constants.hpp"

namespace zappy::server::game {

/**
 * @brief Holds the per-resource spawn densities defined by the subject.
 */
class ResourceDensity {
public:
    /**
     * @brief Density factor used to seed a resource over the whole map.
     *
     * @param type resource to query
     * @return double the canonical density
     */
    static constexpr double densityFor(ResourceType type) noexcept
    {
        return DENSITIES[static_cast<std::size_t>(type)];
    }

private:
    static constexpr std::array<double, RESOURCE_COUNT> DENSITIES = {
        0.5, 0.3, 0.15, 0.1, 0.1, 0.08, 0.05
    };
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_RESOURCEDENSITY_HPP_ */
