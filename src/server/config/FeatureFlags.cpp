/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** FeatureFlags implementation
*/

#include <algorithm>

#include "server/config/FeatureFlags.hpp"
#include "server/config/FeatureFlagInfo.hpp"

/**
 * @brief Build a registry with every flag disabled.
 */
zappy::server::config::FeatureFlags::FeatureFlags() noexcept
    : _enabled{}
{
}

/**
 * @brief Whether @p flag is currently enabled.
 *
 * @param flag flag to query
 * @return bool true if enabled
 */
bool zappy::server::config::FeatureFlags::isEnabled(
    FeatureFlag flag) const noexcept
{
    return _enabled[static_cast<std::size_t>(flag)];
}

/**
 * @brief Enable @p flag.
 *
 * @param flag flag to turn on
 */
void zappy::server::config::FeatureFlags::enable(FeatureFlag flag) noexcept
{
    _enabled[static_cast<std::size_t>(flag)] = true;
}

/**
 * @brief Disable @p flag.
 *
 * @param flag flag to turn off
 */
void zappy::server::config::FeatureFlags::disable(FeatureFlag flag) noexcept
{
    _enabled[static_cast<std::size_t>(flag)] = false;
}

/**
 * @brief Set @p flag to @p value.
 *
 * @param flag flag to update
 * @param value new state
 */
void zappy::server::config::FeatureFlags::set(
    FeatureFlag flag, bool value) noexcept
{
    _enabled[static_cast<std::size_t>(flag)] = value;
}

/**
 * @brief Resolve a wire-protocol name to its flag.
 *
 * @param name lowercase flag name
 * @return std::optional<FeatureFlag> the flag, or nullopt if unknown
 */
std::optional<zappy::server::config::FeatureFlag>
zappy::server::config::FeatureFlags::fromName(std::string_view name) noexcept
{
    const auto it = std::find_if(FEATURE_FLAG_INFOS.begin(),
        FEATURE_FLAG_INFOS.end(),
        [name](const FeatureFlagInfo &info) noexcept {
            return info.name == name;
        });

    return it == FEATURE_FLAG_INFOS.end() ? std::nullopt
        : std::optional<FeatureFlag>(
            static_cast<FeatureFlag>(it - FEATURE_FLAG_INFOS.begin()));
}

/**
 * @brief Wire-protocol name of @p flag.
 *
 * @param flag flag to name
 * @return std::string_view the lowercase name
 */
std::string_view zappy::server::config::FeatureFlags::toName(
    FeatureFlag flag) noexcept
{
    return FEATURE_FLAG_INFOS[static_cast<std::size_t>(flag)].name;
}

/**
 * @brief Snapshot every flag and its current state, in enum order.
 *
 * @return std::vector<std::pair<FeatureFlag, bool>> the flag states
 */
std::vector<std::pair<zappy::server::config::FeatureFlag, bool>>
zappy::server::config::FeatureFlags::snapshot() const
{
    std::vector<std::pair<FeatureFlag, bool>> result;

    result.reserve(FEATURE_FLAG_COUNT);
    for (std::size_t i = 0; i < FEATURE_FLAG_COUNT; ++i)
        result.emplace_back(static_cast<FeatureFlag>(i), _enabled[i]);
    return result;
}
