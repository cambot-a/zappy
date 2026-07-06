/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Server configuration produced by the CLI parser
*/

#ifndef CLI_SERVERCONFIG_HPP_
    #define CLI_SERVERCONFIG_HPP_

    #include <cstdint>
    #include <string>
    #include <utility>
    #include <vector>

    #include "server/config/FeatureFlag.hpp"

namespace zappy::server::cli {

/**
 * @brief Immutable validated configuration consumed by the server at boot.
 */
class ServerConfig {
public:
    /**
     * @brief Construct a configuration from validated values.
     *
     * @param port listening port (1..65535)
     * @param width world width in tiles (> 0)
     * @param height world height in tiles (> 0)
     * @param teamNames unique team names (at least one, no "GRAPHIC")
     * @param clientsPerTeam authorised clients per team (> 0)
     * @param frequency reciprocal of the time unit (> 0)
     * @param enableEvents bonus: dynamic events subsystem
     * @param enableBiomes bonus: biome generation
     * @param enableAdmin bonus: admin console
     * @param enableProfile bonus: player profile command
     * @param adminPassword bonus: admin console password (empty if none)
     */
    ServerConfig(std::uint16_t port, int width, int height,
        std::vector<std::string> teamNames, int clientsPerTeam, int frequency,
        bool enableEvents = false, bool enableBiomes = false,
        bool enableAdmin = false, bool enableProfile = false,
        std::string adminPassword = {})
        : _port(port), _width(width), _height(height),
        _teamNames(std::move(teamNames)), _clientsPerTeam(clientsPerTeam),
        _frequency(frequency), _enableEvents(enableEvents),
        _enableBiomes(enableBiomes), _enableAdmin(enableAdmin),
        _enableProfile(enableProfile), _adminPassword(std::move(adminPassword))
    {
    }

    /**
     * @brief Listening port number.
     *
     * @return std::uint16_t the port
     */
    std::uint16_t port() const noexcept { return _port; }

    /**
     * @brief World width in tiles.
     *
     * @return int the width
     */
    int width() const noexcept { return _width; }

    /**
     * @brief World height in tiles.
     *
     * @return int the height
     */
    int height() const noexcept { return _height; }

    /**
     * @brief Immutable list of unique team names.
     *
     * @return const std::vector<std::string>& reference to the team names
     */
    const std::vector<std::string> &teamNames() const noexcept { return _teamNames; }

    /**
     * @brief Initial authorised clients per team.
     *
     * @return int the number of clients per team
     */
    int clientsPerTeam() const noexcept { return _clientsPerTeam; }

    /**
     * @brief Reciprocal of the time unit for action execution.
     *
     * @return int the frequency
     */
    int frequency() const noexcept { return _frequency; }

    /**
     * @brief Whether the bonus events subsystem is enabled.
     *
     * @return true if the events subsystem is enabled
     * @return false otherwise
     */
    bool eventsEnabled() const noexcept { return _enableEvents; }

    /**
     * @brief Whether the bonus biomes subsystem is enabled.
     *
     * @return true if the biomes subsystem is enabled
     * @return false otherwise
     */
    bool biomesEnabled() const noexcept { return _enableBiomes; }

    /**
     * @brief Whether the bonus admin subsystem is enabled.
     *
     * @return true if the admin subsystem is enabled
     * @return false otherwise
     */
    bool adminEnabled() const noexcept { return _enableAdmin; }

    /**
     * @brief Whether the bonus player profile command is enabled.
     *
     * @return true if the profile command is enabled
     * @return false otherwise
     */
    bool profileEnabled() const noexcept { return _enableProfile; }

    /**
     * @brief Configured admin password.
     *
     * @return const std::string& reference to the password (empty if none provided)
     */
    const std::string &adminPassword() const noexcept { return _adminPassword; }

    /**
     * @brief Feature flags requested on the command line, in enum order.
     *
     * @return std::vector<config::FeatureFlag> the flags to enable at boot
     */
    std::vector<config::FeatureFlag> initialEnabledFlags() const
    {
        std::vector<config::FeatureFlag> flags;

        if (_enableEvents)
            flags.push_back(config::FeatureFlag::EVENTS);
        if (_enableBiomes)
            flags.push_back(config::FeatureFlag::BIOMES);
        if (_enableAdmin)
            flags.push_back(config::FeatureFlag::ADMIN);
        if (_enableProfile)
            flags.push_back(config::FeatureFlag::PROFILE);
        return flags;
    }

private:
    const std::uint16_t _port;
    const int _width;
    const int _height;
    const std::vector<std::string> _teamNames;
    const int _clientsPerTeam;
    const int _frequency;
    const bool _enableEvents;
    const bool _enableBiomes;
    const bool _enableAdmin;
    const bool _enableProfile;
    const std::string _adminPassword;
};

}

#endif /* !CLI_SERVERCONFIG_HPP_ */
