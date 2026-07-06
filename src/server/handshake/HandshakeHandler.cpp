/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** HandshakeHandler implementation
*/

#include <string>

#include "server/handshake/HandshakeHandler.hpp"

/**
 * @brief Bind the handler to the configuration, world and food scheduler.
 *
 * @param config immutable server configuration
 * @param world mutable game world
 * @param foodScheduler per-player food consumption driver
 */
zappy::server::handshake::HandshakeHandler::HandshakeHandler(
    const cli::ServerConfig &config, game::World &world,
    game::FoodScheduler &foodScheduler)
    : _config(config), _world(world), _foodScheduler(foodScheduler)
{}

/**
 * @brief Strip leading and trailing ASCII whitespace and carriage returns.
 *
 * @param s raw input
 * @return std::string trimmed copy
 */
std::string zappy::server::handshake::HandshakeHandler::trim(
    const std::string &s)
{
    const auto first = s.find_first_not_of(" \t\r");
    const auto last = s.find_last_not_of(" \t\r");
    return (first == std::string::npos) ? "" : s.substr(first, last - first + 1);
}

/**
 * @brief Promote @p client to GUI; no further bytes are queued here.
 *
 * @param client HANDSHAKE-state client
 * @return HandshakeResult always PROMOTED
 */
zappy::server::handshake::HandshakeResult
zappy::server::handshake::HandshakeHandler::promoteToGui(
    client::Client &client)
{
    client.promote(client::ClientState::GUI);
    return HandshakeResult::PROMOTED;
}

/**
 * @brief Attempt to promote @p client to AI for team @p name.
 *
 * Queues "ko" and returns DROP when the team is unknown.
 * Otherwise delegates to the egg-consuming spawn path.
 *
 * @param client HANDSHAKE-state client
 * @param name trimmed team name
 * @return HandshakeResult PROMOTED or DROP
 */
zappy::server::handshake::HandshakeResult
zappy::server::handshake::HandshakeHandler::promoteToAi(
    client::Client &client, const std::string &name)
{
    HandshakeResult result = HandshakeResult::DROP;
    if (!_world.hasTeam(name))
        client.buffer().queue_message("ko");
    else
        result = promoteValidAi(client, name);
    return result;
}

/**
 * @brief Consume a waiting egg, spawn the player and queue the reply.
 *
 * Queues "ko" and returns DROP when no egg of @p name is waiting.
 *
 * @param client client passing team validation
 * @param name validated team name
 * @return HandshakeResult PROMOTED or DROP
 */
zappy::server::handshake::HandshakeResult
zappy::server::handshake::HandshakeHandler::promoteValidAi(
    client::Client &client, const std::string &name)
{
    const std::optional<int> eggId = _world.pickRandomWaitingEgg(name, _rng);
    HandshakeResult result = HandshakeResult::DROP;
    if (!eggId)
        client.buffer().queue_message("ko");
    else
        result = spawnFromEgg(client, name, *eggId);
    return result;
}

/**
 * @brief Hatch and remove @p eggId, spawn the player and queue the reply.
 *
 * @param client client passing the egg lookup
 * @param name validated team name
 * @param eggId waiting egg to consume
 * @return HandshakeResult always PROMOTED
 */
zappy::server::handshake::HandshakeResult
zappy::server::handshake::HandshakeHandler::spawnFromEgg(
    client::Client &client, const std::string &name, int eggId)
{
    const game::Position eggPos = _world.egg(eggId).position();
    _world.hatchEgg(eggId);
    _world.removeEgg(eggId);
    const int playerId = _world.addPlayer(name, eggPos, randomOrientation());
    client.promote(client::ClientState::AI);
    client.aiData().teamName = name;
    client.aiData().playerId = playerId;
    _foodScheduler.startConsumption(playerId);
    client.buffer().queue_message(
        std::to_string(_world.waitingEggCount(name)));
    client.buffer().queue_message(std::to_string(_config.width()) + " " +
        std::to_string(_config.height()));
    return HandshakeResult::PROMOTED;
}

/**
 * @brief Draw a uniformly random cardinal orientation.
 *
 * @return game::Orientation the spawn orientation
 */
zappy::server::game::Orientation
zappy::server::handshake::HandshakeHandler::randomOrientation()
{
    static constexpr std::array<game::Orientation, 4> orientations = {
        game::Orientation::NORTH, game::Orientation::EAST,
        game::Orientation::SOUTH, game::Orientation::WEST};
    std::uniform_int_distribution<std::size_t> dist(
        0, orientations.size() - 1);
    return orientations[dist(_rng)];
}

/**
 * @brief Route @p line to the appropriate promotion path.
 *
 * Trims whitespace first; an empty line is rejected immediately.
 *
 * @param client HANDSHAKE-state client
 * @param line raw line from the network (no trailing newline)
 * @return HandshakeResult PROMOTED or DROP
 */
zappy::server::handshake::HandshakeResult
zappy::server::handshake::HandshakeHandler::handle(
    client::Client &client, const std::string &line)
{
    const std::string name = trim(line);
    HandshakeResult result = HandshakeResult::DROP;
    if (name.empty())
        client.buffer().queue_message("ko");
    else if (name == "GRAPHIC")
        result = promoteToGui(client);
    else
        result = promoteToAi(client, name);
    return result;
}
