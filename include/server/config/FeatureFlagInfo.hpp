/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Constexpr wire-protocol metadata for each feature flag
*/

#ifndef SERVER_CONFIG_FEATUREFLAGINFO_HPP_
    #define SERVER_CONFIG_FEATUREFLAGINFO_HPP_

    #include <array>
    #include <string_view>

    #include "server/config/FeatureFlag.hpp"

namespace zappy::server::config {

/**
 * @brief Static metadata for a feature flag.
 */
struct FeatureFlagInfo {
    std::string_view name;
};

/**
 * @brief Flag metadata indexed by static_cast<std::size_t>(FeatureFlag).
 */
inline constexpr std::array<FeatureFlagInfo, FEATURE_FLAG_COUNT>
    FEATURE_FLAG_INFOS = {{
        {"events"},
        {"biomes"},
        {"admin"},
        {"profile"}
    }};

} // namespace zappy::server::config

#endif /* !SERVER_CONFIG_FEATUREFLAGINFO_HPP_ */
