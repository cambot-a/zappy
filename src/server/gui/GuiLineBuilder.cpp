/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GuiLineBuilder implementation
*/

#include "server/gui/GuiLineBuilder.hpp"

/**
 * @brief Construct a new Gui Line Builder object.
 *
 * @param world read-only reference to the game world
 */
zappy::server::gui::GuiLineBuilder::GuiLineBuilder(
    const game::World &world) noexcept
    : _world(world)
{
}

/**
 * @brief Build the bct protocol string for a tile.
 *
 * @param x tile horizontal coordinate
 * @param y tile vertical coordinate
 * @return std::string the formatted "bct X Y q0..q6" line
 */
std::string zappy::server::gui::GuiLineBuilder::bct(int x, int y) const
{
    const game::Tile &tile = _world.tileAt(game::Position(x, y));
    std::string line = "bct " + std::to_string(x) + " " + std::to_string(y);

    for (std::size_t i = 0; i < game::RESOURCE_COUNT; ++i) {
        line += " " + std::to_string(
            tile.resource(static_cast<game::ResourceType>(i)));
    }
    return line;
}

/**
 * @brief Build the ppo protocol string for a player.
 *
 * @param playerId player ID
 * @return std::string the formatted "ppo #n X Y O" line
 */
std::string zappy::server::gui::GuiLineBuilder::ppo(int playerId) const
{
    const game::Player &player = _world.player(playerId);

    return "ppo #" + std::to_string(playerId) + " "
        + std::to_string(player.position().x()) + " "
        + std::to_string(player.position().y()) + " "
        + std::to_string(static_cast<int>(player.orientation()));
}

/**
 * @brief Build the plv protocol string for a player.
 *
 * @param playerId player ID
 * @return std::string the formatted "plv #n L" line
 */
std::string zappy::server::gui::GuiLineBuilder::plv(int playerId) const
{
    return "plv #" + std::to_string(playerId) + " "
        + std::to_string(_world.player(playerId).level());
}

/**
 * @brief Build the pin protocol string for a player.
 *
 * @param playerId player ID
 * @return std::string the formatted "pin #n X Y q0..q6" line
 */
std::string zappy::server::gui::GuiLineBuilder::pin(int playerId) const
{
    const game::Player &player = _world.player(playerId);
    std::string line = "pin #" + std::to_string(playerId) + " "
        + std::to_string(player.position().x()) + " "
        + std::to_string(player.position().y());

    for (std::size_t i = 0; i < game::RESOURCE_COUNT; ++i) {
        line += " " + std::to_string(
            player.resource(static_cast<game::ResourceType>(i)));
    }
    return line;
}

/**
 * @brief Build the "ppf #n N X Y O L q0..q6" aggregated profile line.
 *
 * @param playerId player ID
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::ppf(int playerId) const
{
    const game::Player &player = _world.player(playerId);
    std::string line = "ppf #" + std::to_string(playerId) + " "
        + player.team() + " " + std::to_string(player.position().x()) + " "
        + std::to_string(player.position().y()) + " "
        + std::to_string(static_cast<int>(player.orientation())) + " "
        + std::to_string(player.level());

    for (std::size_t i = 0; i < game::RESOURCE_COUNT; ++i) {
        line += " " + std::to_string(
            player.resource(static_cast<game::ResourceType>(i)));
    }
    return line;
}

std::string zappy::server::gui::GuiLineBuilder::pnw(int playerId) const
{
    const game::Player &player = _world.player(playerId);

    return "pnw #" + std::to_string(playerId) + " "
        + std::to_string(player.position().x()) + " "
        + std::to_string(player.position().y()) + " "
        + std::to_string(static_cast<int>(player.orientation())) + " "
        + std::to_string(player.level()) + " "
        + player.team();
}

std::string zappy::server::gui::GuiLineBuilder::pex(int playerId) const
{
    return "pex #" + std::to_string(playerId);
}

std::string zappy::server::gui::GuiLineBuilder::pbc(
    int playerId, const std::string &message) const
{
    return "pbc #" + std::to_string(playerId) + " " + message;
}

std::string zappy::server::gui::GuiLineBuilder::pic(int initiatorId, int level,
    const std::vector<int> &participants) const
{
    const game::Player &initiator = _world.player(initiatorId);
    std::string line = "pic "
        + std::to_string(initiator.position().x()) + " "
        + std::to_string(initiator.position().y()) + " "
        + std::to_string(level + 1);

    for (int id : participants) {
        line += " #" + std::to_string(id);
    }
    return line;
}

std::string zappy::server::gui::GuiLineBuilder::pie(
    game::Position pos, bool success) const
{
    return "pie " + std::to_string(pos.x()) + " "
        + std::to_string(pos.y()) + " "
        + (success ? "1" : "0");
}

std::string zappy::server::gui::GuiLineBuilder::pfk(int playerId) const
{
    return "pfk #" + std::to_string(playerId);
}

std::string zappy::server::gui::GuiLineBuilder::pdr(
    int playerId, game::ResourceType resource) const
{
    return "pdr #" + std::to_string(playerId) + " "
        + std::to_string(static_cast<int>(resource));
}

std::string zappy::server::gui::GuiLineBuilder::pgt(
    int playerId, game::ResourceType resource) const
{
    return "pgt #" + std::to_string(playerId) + " "
        + std::to_string(static_cast<int>(resource));
}

std::string zappy::server::gui::GuiLineBuilder::pdi(int playerId) const
{
    return "pdi #" + std::to_string(playerId);
}

/**
 * @brief Build the "enw #e #n X Y" line for a laid egg.
 *
 * @param eggId egg ID
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::enw(int eggId) const
{
    const game::Egg &egg = _world.egg(eggId);
    std::string playerStr = (egg.layingPlayerId() >= 0) ?
        ("#" + std::to_string(egg.layingPlayerId())) : "-1";

    return "enw #" + std::to_string(eggId) + " " + playerStr + " "
        + std::to_string(egg.position().x()) + " "
        + std::to_string(egg.position().y());
}

/**
 * @brief Build the "ebo #e" line for egg used by AI connection.
 *
 * @param eggId egg ID
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::ebo(int eggId) const
{
    return "ebo #" + std::to_string(eggId);
}

/**
 * @brief Build the "edi #e" line for egg death.
 *
 * @param eggId egg ID
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::edi(int eggId) const
{
    return "edi #" + std::to_string(eggId);
}

/**
 * @brief Build the "seg N" line for end of game.
 *
 * @param teamName name of the winning team
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::seg(
    const std::string &teamName) const
{
    return "seg " + teamName;
}

/**
 * @brief Build the "smg M" line for a server message.
 *
 * @param message the message string
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::smg(
    const std::string &message) const
{
    return "smg " + message;
}

/**
 * @brief Build the "evt_flood_tile X Y on|off" custom flood line.
 *
 * @param x tile horizontal coordinate
 * @param y tile vertical coordinate
 * @param flooded true for "on", false for "off"
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::evtFloodTile(
    int x, int y, bool flooded) const
{
    return "evt_flood_tile " + std::to_string(x) + " " + std::to_string(y)
        + (flooded ? " on" : " off");
}

/**
 * @brief Build the "evt_biome_set X Y <name>" custom biome line.
 *
 * @param x tile horizontal coordinate
 * @param y tile vertical coordinate
 * @param biome the tile's biome
 * @return std::string the formatted line
 */
std::string zappy::server::gui::GuiLineBuilder::evtBiomeSet(
    int x, int y, game::Biome biome) const
{
    return "evt_biome_set " + std::to_string(x) + " " + std::to_string(y)
        + " " + std::string(game::biomeInfoFor(biome).name);
}
