/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Runtime registry of enabled bonus feature flags
*/

#ifndef SERVER_CONFIG_FEATUREFLAGS_HPP_
    #define SERVER_CONFIG_FEATUREFLAGS_HPP_

    #include <array>
    #include <optional>
    #include <string_view>
    #include <utility>
    #include <vector>

    #include "server/config/FeatureFlag.hpp"

namespace zappy::server::config {

/**
 * @brief Holds the on/off state of every bonus feature flag.
 *
 * All flags default to disabled (strict mode). The GUI admin and the CLI
 * may toggle them; the server is single-threaded so no locking is needed.
 */
class FeatureFlags {
public:
    /**
     * @brief Build a registry with every flag disabled.
     */
    FeatureFlags() noexcept;

    /**
     * @brief Whether @p flag is currently enabled.
     *
     * @param flag flag to query
     * @return bool true if enabled
     */
    [[nodiscard]] bool isEnabled(FeatureFlag flag) const noexcept;

    /**
     * @brief Enable @p flag.
     *
     * @param flag flag to turn on
     */
    void enable(FeatureFlag flag) noexcept;

    /**
     * @brief Disable @p flag.
     *
     * @param flag flag to turn off
     */
    void disable(FeatureFlag flag) noexcept;

    /**
     * @brief Set @p flag to @p value.
     *
     * @param flag flag to update
     * @param value new state
     */
    void set(FeatureFlag flag, bool value) noexcept;

    /**
     * @brief Resolve a wire-protocol name to its flag.
     *
     * @param name lowercase flag name
     * @return std::optional<FeatureFlag> the flag, or nullopt if unknown
     */
    [[nodiscard]] static std::optional<FeatureFlag> fromName(
        std::string_view name) noexcept;

    /**
     * @brief Wire-protocol name of @p flag.
     *
     * @param flag flag to name
     * @return std::string_view the lowercase name
     */
    [[nodiscard]] static std::string_view toName(FeatureFlag flag) noexcept;

    /**
     * @brief Snapshot every flag and its current state, in enum order.
     *
     * @return std::vector<std::pair<FeatureFlag, bool>> the flag states
     */
    [[nodiscard]] std::vector<std::pair<FeatureFlag, bool>> snapshot() const;

private:
    std::array<bool, FEATURE_FLAG_COUNT> _enabled;
};

} // namespace zappy::server::config

#endif /* !SERVER_CONFIG_FEATUREFLAGS_HPP_ */
