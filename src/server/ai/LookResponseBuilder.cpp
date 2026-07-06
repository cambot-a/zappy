/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** LookResponseBuilder implementation
*/

#include <cstddef>
#include <vector>

#include "server/ai/LookResponseBuilder.hpp"
#include "server/game/Player.hpp"
#include "server/game/VisionCone.hpp"

/**
 * @brief Bind the builder to the world it reads from.
 *
 * @param world game world to inspect (read-only)
 */
zappy::server::ai::LookResponseBuilder::LookResponseBuilder(
    const game::World &world)
    : _world(world)
{}

/**
 * @brief Format the Look response for @p playerId.
 *
 * @param playerId player whose vision is rendered
 * @return std::string the bracketed, comma-separated tile list
 */
std::string zappy::server::ai::LookResponseBuilder::buildFor(int playerId) const
{
    const game::Player &player = _world.player(playerId);
    const std::vector<game::Position> tiles = game::VisionCone::tilesFor(
        player.position(), player.orientation(), player.level(),
        _world.width(), _world.height());
    std::string out = "[";
    bool firstTile = true;
    for (const game::Position &pos : tiles) {
        if (!firstTile)
            out += ", ";
        firstTile = false;
        appendTile(out, pos);
    }
    out += "]";
    return out;
}

/**
 * @brief Append one tile's contents (players then resources) to @p out.
 *
 * @param out string being built
 * @param pos tile position to render
 */
void zappy::server::ai::LookResponseBuilder::appendTile(std::string &out,
    game::Position pos) const
{
    const game::Tile &tile = _world.tileAt(pos);
    bool first = true;
    appendPlayers(out, tile, first);
    appendResources(out, tile, first);
}

/**
 * @brief Append a "player" token per player on @p tile.
 *
 * @param out string being built
 * @param tile tile to read
 * @param first space-tracking flag, updated in place
 */
void zappy::server::ai::LookResponseBuilder::appendPlayers(std::string &out,
    const game::Tile &tile, bool &first) const
{
    for ([[maybe_unused]] const int id : tile.playerIds())
        appendToken(out, "player", first);
}

/**
 * @brief Append one resource-name token per resource unit on @p tile.
 *
 * @param out string being built
 * @param tile tile to read
 * @param first space-tracking flag, updated in place
 */
void zappy::server::ai::LookResponseBuilder::appendResources(std::string &out,
    const game::Tile &tile, bool &first) const
{
    for (std::size_t i = 0; i < game::RESOURCE_COUNT; ++i)
        for (int n = tile.resource(static_cast<game::ResourceType>(i)); n > 0;
            --n)
            appendToken(out, game::RESOURCE_NAMES[i], first);
}

/**
 * @brief Append @p token, prefixed by a space unless it is the first.
 *
 * @param out string being built
 * @param token token to append
 * @param first space-tracking flag, updated in place
 */
void zappy::server::ai::LookResponseBuilder::appendToken(std::string &out,
    std::string_view token, bool &first) const
{
    if (!first)
        out += ' ';
    out += token;
    first = false;
}
