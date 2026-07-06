/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Processes the first line of a HANDSHAKE-state client
*/

#ifndef HANDSHAKE_HANDSHAKEHANDLER_HPP_
    #define HANDSHAKE_HANDSHAKEHANDLER_HPP_

    #include <array>
    #include <random>
    #include <string>

    #include "server/cli/ServerConfig.hpp"
    #include "server/client/Client.hpp"
    #include "server/game/FoodScheduler.hpp"
    #include "server/game/World.hpp"

namespace zappy::server::handshake {

/**
 * @brief Outcome of a single HandshakeHandler::handle() call.
 */
enum class HandshakeResult { PROMOTED, DROP };

/**
 * @brief Stateless dispatcher for the initial client negotiation.
 *
 * Owns references to the server config and world; therefore
 * non-copyable and non-movable.
 */
class HandshakeHandler {
public:
    /**
     * @brief Bind to the validated configuration and the live world.
     *
     * @param config immutable server configuration
     * @param world mutable game world
     * @param foodScheduler per-player food consumption driver
     */
    HandshakeHandler(const cli::ServerConfig &config,
        game::World &world, game::FoodScheduler &foodScheduler);

    HandshakeHandler(const HandshakeHandler &) = delete;
    HandshakeHandler &operator=(const HandshakeHandler &) = delete;
    HandshakeHandler(HandshakeHandler &&) = delete;
    HandshakeHandler &operator=(HandshakeHandler &&) = delete;

    /**
     * @brief Process @p line received from a HANDSHAKE-state @p client.
     *
     * Trims leading/trailing whitespace, then:
     * - "GRAPHIC"   promotes to GUI, returns PROMOTED.
     * - Known team with a waiting egg   consumes the egg, promotes to AI,
     *   queues `<waiting-eggs>\n` and `<w> <h>\n`, returns PROMOTED.
     * - Anything else   queues `ko\n`, returns DROP.
     *
     * @param client HANDSHAKE-state client (mutated in place)
     * @param line raw line received from the network (no trailing newline)
     * @return HandshakeResult PROMOTED or DROP
     */
    [[nodiscard]] HandshakeResult handle(client::Client &client,
        const std::string &line);

private:
    /**
     * @brief Strip leading and trailing ASCII spaces, tabs and carriage returns.
     *
     * @param s raw input string
     * @return std::string trimmed copy
     */
    [[nodiscard]] static std::string trim(const std::string &s);

    /**
     * @brief Promote @p client to GUI state.
     *
     * @param client client to promote
     * @return HandshakeResult always PROMOTED
     */
    [[nodiscard]] HandshakeResult promoteToGui(client::Client &client);

    /**
     * @brief Validate @p name against the world and promote to AI.
     *
     * Queues `ko\n` and returns DROP if the team is unknown.
     * Otherwise delegates to the egg-consuming spawn path.
     *
     * @param client client to promote
     * @param name trimmed team name supplied by the client
     * @return HandshakeResult PROMOTED or DROP
     */
    [[nodiscard]] HandshakeResult promoteToAi(client::Client &client,
        const std::string &name);

    /**
     * @brief Pick a waiting egg, spawn the player and queue the AI reply.
     *
     * Queues `ko\n` and returns DROP if no egg of @p name is waiting.
     *
     * @param client client passing the team validation
     * @param name validated team name
     * @return HandshakeResult PROMOTED or DROP
     */
    [[nodiscard]] HandshakeResult promoteValidAi(client::Client &client,
        const std::string &name);

    /**
     * @brief Hatch and remove an egg, spawn the player and queue the reply.
     *
     * @param client client passing the egg lookup
     * @param name validated team name
     * @param eggId waiting egg to consume
     * @return HandshakeResult always PROMOTED
     */
    [[nodiscard]] HandshakeResult spawnFromEgg(client::Client &client,
        const std::string &name, int eggId);

    /**
     * @brief Draw a uniformly random cardinal orientation.
     *
     * @return game::Orientation the spawn orientation
     */
    [[nodiscard]] game::Orientation randomOrientation();

    const cli::ServerConfig &_config;
    game::World &_world;
    game::FoodScheduler &_foodScheduler;
    std::mt19937_64 _rng{std::random_device{}()};
};

} // namespace zappy::server::handshake

#endif /* !HANDSHAKE_HANDSHAKEHANDLER_HPP_ */
