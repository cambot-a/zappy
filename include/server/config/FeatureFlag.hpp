/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Enumeration of the runtime-toggleable bonus feature flags
*/

#ifndef SERVER_CONFIG_FEATUREFLAG_HPP_
    #define SERVER_CONFIG_FEATUREFLAG_HPP_

    #include <cstddef>

namespace zappy::server::config {

/**
 * @brief Bonus subsystems gated behind a runtime flag (strict mode by default).
 */
enum class FeatureFlag {
    EVENTS,
    BIOMES,
    ADMIN,
    PROFILE
};

constexpr std::size_t FEATURE_FLAG_COUNT = 4;

} // namespace zappy::server::config

#endif /* !SERVER_CONFIG_FEATUREFLAG_HPP_ */
