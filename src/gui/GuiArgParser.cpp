/*
** EPITECH PROJECT, 2026
** GuiArgParser.cpp
** File description:
** Implementation of the zappy_server command line parser
*/

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "gui/GuiArgParser.hpp"
#include "gui/ipc/SerializationError.hpp"
#include "gui/Constants.hpp"
#include "gui/ipc/ServerInfo.hpp"

namespace zappy::gui {

const std::string &GuiArgParser::takeValue(const std::vector<std::string> &args,
    std::size_t &index, std::string_view flag)
{
    if (index + 1 >= args.size())
        throw ipc::SerializationError("missing value for " + std::string(flag));
    return args[++index];
}

/// @brief Parse a whole token as a base-10 integer; reject any extra char.
long GuiArgParser::takeInteger(const std::vector<std::string> &args,
    std::size_t &index, std::string_view flag)
{
    const std::string &token = takeValue(args, index, flag);
    std::size_t consumed = 0;
    long value = 0;

    try {
        value = std::stol(token, &consumed);
    } catch (const std::exception &) {
        consumed = 0;
    }
    if (consumed == 0 || consumed != token.size())
        throw ipc::SerializationError("invalid integer for " + std::string(flag)
            + ": '" + token + "'");
    return value;
}

void GuiArgParser::collect(const std::vector<std::string> &args, Builder &out)
{
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string &arg = args[i];

        if (arg == "-p") {
            out.port = takeInteger(args, i, arg);
        } else if (arg == "-h") {
            out.hostname = takeValue(args, i, arg);
        }
    }
}

ipc::ServerInfo GuiArgParser::finalize(const Builder &b)
{
    const long port = b.port.value_or(kDefaultPort);
    if (port <= 0 || port > std::numeric_limits<std::uint16_t>::max())
        throw ipc::SerializationError("-p port must be in range 1..65535");
    const std::string server_hostname = std::string(b.hostname.value_or(kDefaultNamespace));

    return ipc::ServerInfo(static_cast<std::uint16_t>(port), server_hostname);
}

std::string GuiArgParser::usage()
{
    static std::string gui_help =
    "USAGE: ./zappy_gui -p port -h machine\n"
    "./zappy_gui\n"
    "--help  help flag\n"
    "-p      port port number\n"
    "-h      machine hostname of the server\n";
    return gui_help;
}

ipc::ServerInfo GuiArgParser::parse(int argc, const char *const argv[])
{
    std::vector<std::string> args;
    Builder builder;

    args.reserve(static_cast<std::size_t>(argc < 0 ? 0 : argc));
    for (int i = 0; i < argc; ++i)
        args.emplace_back(argv[i]);
    collect(args, builder);
    return finalize(builder);
}

bool GuiArgParser::wantsHelp(int argc, const char *const argv[])
{
    for (int i = 1; i < argc; ++i)
        if (std::string_view(argv[i]) == "--help")
            return true;
    return false;
}

} // namespace zappy::gui
