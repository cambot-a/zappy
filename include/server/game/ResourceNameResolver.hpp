/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Reverse lookup from a resource name to its ResourceType
*/

#ifndef SERVER_GAME_RESOURCENAMERESOLVER_HPP_
    #define SERVER_GAME_RESOURCENAMERESOLVER_HPP_

    #include <algorithm>
    #include <iterator>
    #include <optional>
    #include <string_view>

    #include "server/game/Constants.hpp"

namespace zappy::server::game {

/**
 * @brief Maps a resource name string back to its ResourceType.
 */
class ResourceNameResolver {
public:
    /**
     * @brief Resolve @p name to its ResourceType.
     *
     * @param name resource name to look up (case sensitive, no trimming)
     * @return std::optional<ResourceType> the type, or nullopt if unknown
     */
    [[nodiscard]] static std::optional<ResourceType> resolve(
        std::string_view name) noexcept
    {
        const auto begin = RESOURCE_NAMES.begin();
        const auto end = RESOURCE_NAMES.end();
        const auto it = std::find(begin, end, name);
        std::optional<ResourceType> result;

        if (it != end)
            result = static_cast<ResourceType>(std::distance(begin, it));
        return result;
    }
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_RESOURCENAMERESOLVER_HPP_ */
