/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Command line parser turning argc/argv into a ServerConfig
*/

#ifndef CLI_CLIPARSER_HPP_
    #define CLI_CLIPARSER_HPP_

    #include <cstddef>
    #include <optional>
    #include <string>
    #include <string_view>
    #include <vector>

    #include "server/cli/ServerConfig.hpp"

namespace zappy::server::cli {

/**
 * @brief Parses the zappy_server command line into a @ref ServerConfig.
 */
class CliParser {
public:
    CliParser() = default;

    /**
     * @brief Parse a command line into a validated configuration.
     *
     * @param argc argument count from @c main()
     * @param argv argument vector from @c main()
     * @return std::optional<ServerConfig> the configuration, or std::nullopt when @c --help was asked
     * @throws CliParseError on any invalid input
     */
    std::optional<ServerConfig> parse(int argc, const char *const argv[]) const;

    /**
     * @brief The usage / help message.
     *
     * @return std::string the usage string, newline-terminated
     */
    static std::string usage();

private:
    /**
     * @brief Accumulator filled while scanning the argument vector.
     */
    struct Builder {
        std::optional<long> port;
        std::optional<long> width;
        std::optional<long> height;
        std::vector<std::string> teamNames;
        bool sawTeamFlag = false;
        std::optional<long> clientsPerTeam;
        std::optional<long> frequency;

        bool enableEvents = false;
        bool enableBiomes = false;
        bool enableAdmin = false;
        bool enableProfile = false;
        std::optional<std::string> adminPassword;
    };

    /**
     * @brief Tokenise the arguments into the builder.
     *
     * @param args raw argument tokens (excluding argv[0])
     * @param out builder to populate
     * @throws CliParseError on any invalid token
     */
    void collect(const std::vector<std::string> &args, Builder &out) const;

    /**
     * @brief Validate the builder and build the config.
     *
     * @param b populated builder
     * @return ServerConfig the validated configuration
     * @throws CliParseError if any required field is missing or invalid
     */
    ServerConfig finalize(const Builder &b) const;

    /**
     * @brief Enforce that --enable-admin is paired with a non-empty password.
     *
     * @param b populated builder
     * @throws CliParseError if admin is enabled without an --admin-password
     */
    void validateAdminPassword(const Builder &b) const;

    /**
     * @brief Take the value token following @p flag, advancing @p index.
     *
     * @param args raw argument tokens
     * @param index in/out: current position in @p args, advanced past the value
     * @param flag the flag being parsed (used in error messages)
     * @return const std::string& reference to the consumed value
     * @throws CliParseError if no value follows the flag
     */
    const std::string &takeValue(const std::vector<std::string> &args,
        std::size_t &index, std::string_view flag) const;

    /**
     * @brief Take an integer flag value, advancing @p index.
     *
     * @param args raw argument tokens
     * @param index in/out: current position in @p args, advanced past the value
     * @param flag the flag being parsed (used in error messages)
     * @return long the parsed integer
     * @throws CliParseError if the value is missing or not a valid integer
     */
    long takeInteger(const std::vector<std::string> &args, std::size_t &index,
        std::string_view flag) const;

    /**
     * @brief Take a -n run of team names, advancing @p index.
     *
     * @param args raw argument tokens
     * @param index in/out: current position in @p args, advanced past the last team name
     * @param out builder receiving the team names
     * @throws CliParseError if no team name follows -n
     */
    void takeTeamNames(const std::vector<std::string> &args, std::size_t &index,
        Builder &out) const;
};

}

#endif /* !CLI_CLIPARSER_HPP_ */
