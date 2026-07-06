/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Implementation of the zappy_server command line parser
*/

#include "server/cli/CliParser.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "server/cli/CliParseError.hpp"

namespace zappy::server::cli {

namespace {

constexpr int kDefaultFrequency = 100;
constexpr int kDefaultPort = 4242;
constexpr int kDefaultWidth = 10;
constexpr int kDefaultHeight = 10;
constexpr int kDefaultClientsPerTeam = 5;
constexpr std::string_view kReservedTeamName = "GRAPHIC";

/// @brief Default team names used when -n is omitted entirely.
std::vector<std::string> defaultTeamNames()
{
    return {"team1", "team2"};
}

/// @brief Parse a whole token as a base-10 integer; reject any extra char.
long parseStrictInteger(const std::string &token, std::string_view flag)
{
    std::size_t consumed = 0;
    long value = 0;

    try {
        value = std::stol(token, &consumed);
    } catch (const std::exception &) {
        consumed = 0;
    }
    if (consumed == 0 || consumed != token.size())
        throw CliParseError("invalid integer for " + std::string(flag)
            + ": '" + token + "'");
    return value;
}

/// @brief Require a positive value (> 0) for a flag, or throw.
void requirePositive(long value, std::string_view flag)
{
    if (value <= 0)
        throw CliParseError(std::string(flag) + " must be strictly positive");
}

} // namespace

const std::string &CliParser::takeValue(const std::vector<std::string> &args,
    std::size_t &index, std::string_view flag) const
{
    if (index + 1 >= args.size())
        throw CliParseError("missing value for " + std::string(flag));
    return args[++index];
}

long CliParser::takeInteger(const std::vector<std::string> &args,
    std::size_t &index, std::string_view flag) const
{
    return parseStrictInteger(takeValue(args, index, flag), flag);
}

void CliParser::takeTeamNames(const std::vector<std::string> &args,
    std::size_t &index, Builder &out) const
{
    while (index + 1 < args.size() && args[index + 1][0] != '-') {
        const std::string &name = args[++index];
        if (name.empty())
            throw CliParseError("team name cannot be empty");
        out.teamNames.push_back(name);
    }
}

void CliParser::collect(const std::vector<std::string> &args, Builder &out) const
{
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string &arg = args[i];

        if (arg == "-p")
            out.port = takeInteger(args, i, arg);
        else if (arg == "-x")
            out.width = takeInteger(args, i, arg);
        else if (arg == "-y")
            out.height = takeInteger(args, i, arg);
        else if (arg == "-c")
            out.clientsPerTeam = takeInteger(args, i, arg);
        else if (arg == "-f")
            out.frequency = takeInteger(args, i, arg);
        else if (arg == "-n") {
            out.sawTeamFlag = true;
            takeTeamNames(args, i, out);
        }
        else if (arg == "--enable-events")
            out.enableEvents = true;
        else if (arg == "--enable-biomes")
            out.enableBiomes = true;
        else if (arg == "--enable-admin")
            out.enableAdmin = true;
        else if (arg == "--enable-profile")
            out.enableProfile = true;
        else if (arg == "--admin-password")
            out.adminPassword = takeValue(args, i, arg);
        else
            throw CliParseError("unknown argument: " + arg);
    }
}

void CliParser::validateAdminPassword(const Builder &b) const
{
    if (b.enableAdmin && b.adminPassword.value_or(std::string{}).empty())
        throw CliParseError("admin feature requires --admin-password");
}

ServerConfig CliParser::finalize(const Builder &b) const
{
    const long port = b.port.value_or(kDefaultPort);
    if (port <= 0 || port > std::numeric_limits<std::uint16_t>::max())
        throw CliParseError("-p port must be in range 1..65535");

    const long width = b.width.value_or(kDefaultWidth);
    requirePositive(width, "-x");

    const long height = b.height.value_or(kDefaultHeight);
    requirePositive(height, "-y");

    if (b.sawTeamFlag && b.teamNames.empty())
        throw CliParseError("missing team name after -n");
    std::vector<std::string> teamNames =
        b.teamNames.empty() ? defaultTeamNames() : b.teamNames;
    for (const std::string &name : teamNames) {
        if (name == kReservedTeamName)
            throw CliParseError("team name 'GRAPHIC' is reserved");
        if (std::count(teamNames.begin(), teamNames.end(), name) > 1)
            throw CliParseError("duplicate team name: " + name);
    }

    const long clientsPerTeam = b.clientsPerTeam.value_or(kDefaultClientsPerTeam);
    requirePositive(clientsPerTeam, "-c");

    const long frequency = b.frequency.value_or(kDefaultFrequency);
    requirePositive(frequency, "-f");

    validateAdminPassword(b);
    return ServerConfig(static_cast<std::uint16_t>(port),
        static_cast<int>(width), static_cast<int>(height),
        teamNames, static_cast<int>(clientsPerTeam),
        static_cast<int>(frequency), b.enableEvents, b.enableBiomes,
        b.enableAdmin, b.enableProfile,
        b.adminPassword.value_or(std::string{}));
}

std::optional<ServerConfig> CliParser::parse(int argc,
    const char *const argv[]) const
{
    std::optional<ServerConfig> result;
    std::vector<std::string> args;
    bool helpRequested = false;

    args.reserve(static_cast<std::size_t>(argc < 0 ? 0 : argc));
    for (int i = 0; i < argc; ++i)
        args.emplace_back(argv[i]);
    for (const std::string &arg : args) {
        if (arg == "--help")
            helpRequested = true;
    }
    if (!helpRequested) {
        Builder builder;
        collect(args, builder);
        result.emplace(finalize(builder));
    }
    return result;
}

std::string CliParser::usage()
{
    return
        "USAGE: ./zappy_server -p port -x width -y height -n name1 name2 ... "
        "-c clientsNb -f freq\n"
        "All arguments are optional; defaults are used when omitted.\n"
        "-p port      port number (default: 4242)\n"
        "-x width     width of the world (default: 10)\n"
        "-y height    height of the world (default: 10)\n"
        "-n name1 ... name of the team (default: team1 team2)\n"
        "-c clientsNb number of authorized clients per team (default: 5)\n"
        "-f freq      reciprocal of time unit for execution of actions "
        "(default: 100)\n"
        "--enable-events       (optional) enable dynamic events [bonus]\n"
        "--enable-biomes       (optional) enable biome generation [bonus]\n"
        "--enable-admin        (optional) enable admin console [bonus]\n"
        "--enable-profile      (optional) enable player profile command [bonus]\n"
        "--admin-password <pw> (optional) admin console password [bonus]\n";
}

} // namespace zappy::server::cli
