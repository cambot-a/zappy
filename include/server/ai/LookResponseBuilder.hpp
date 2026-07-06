/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Formats the Look command response for a player
*/

#ifndef SERVER_AI_LOOKRESPONSEBUILDER_HPP_
    #define SERVER_AI_LOOKRESPONSEBUILDER_HPP_

    #include <string>
    #include <string_view>

    #include "server/game/Tile.hpp"
    #include "server/game/World.hpp"

namespace zappy::server::ai {

/**
 * @brief Builds the "[tile, tile, ...]" Look answer from world state.
 */
class LookResponseBuilder {
public:
    /**
     * @brief Bind the builder to the world it reads from.
     *
     * @param world game world to inspect (read-only)
     */
    explicit LookResponseBuilder(const game::World &world);

    /**
     * @brief Format the Look response for @p playerId.
     *
     * @param playerId player whose vision is rendered
     * @return std::string the bracketed, comma-separated tile list
     */
    [[nodiscard]] std::string buildFor(int playerId) const;

private:
    /**
     * @brief Append one tile's contents (players then resources) to @p out.
     *
     * @param out string being built
     * @param pos tile position to render
     */
    void appendTile(std::string &out, game::Position pos) const;

    /**
     * @brief Append a "player" token per player on @p tile.
     *
     * @param out string being built
     * @param tile tile to read
     * @param first space-tracking flag, updated in place
     */
    void appendPlayers(std::string &out, const game::Tile &tile,
        bool &first) const;

    /**
     * @brief Append one resource-name token per resource unit on @p tile.
     *
     * @param out string being built
     * @param tile tile to read
     * @param first space-tracking flag, updated in place
     */
    void appendResources(std::string &out, const game::Tile &tile,
        bool &first) const;

    /**
     * @brief Append @p token, prefixed by a space unless it is the first.
     *
     * @param out string being built
     * @param token token to append
     * @param first space-tracking flag, updated in place
     */
    void appendToken(std::string &out, std::string_view token,
        bool &first) const;

    const game::World &_world;
};

} // namespace zappy::server::ai

#endif /* !SERVER_AI_LOOKRESPONSEBUILDER_HPP_ */
