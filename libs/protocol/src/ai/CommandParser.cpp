/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** CommandParser implementation
*/

#include <algorithm>
#include <cstddef>
#include <string>

#include "protocol/ai/CommandInfo.hpp"
#include "protocol/ai/CommandParser.hpp"

/**
 * @brief Strip leading and trailing spaces, tabs and carriage returns.
 *
 * @param s raw view
 * @return std::string_view trimmed view into @p s, empty if all whitespace
 */
std::string_view zappy::protocol::ai::CommandParser::trim(
    std::string_view s) noexcept
{
    const std::size_t first = s.find_first_not_of(" \t\r");
    const std::size_t last = s.find_last_not_of(" \t\r");
    return first == std::string_view::npos
        ? std::string_view{} : s.substr(first, last - first + 1);
}

/**
 * @brief Parse @p line into a ParsedCommand or reject it.
 *
 * Splits the verb from its argument and accepts the line only when the
 * verb is known and the presence of an argument matches its metadata.
 *
 * @param line raw command line
 * @return std::optional<ParsedCommand> nullopt if unknown or malformed
 */
std::optional<zappy::protocol::ai::ParsedCommand>
zappy::protocol::ai::CommandParser::parse(std::string_view line)
{
    const std::string_view trimmed = trim(line);
    const std::size_t sep = trimmed.find_first_of(" \t\r");
    const std::string_view verb = trimmed.substr(0, sep);
    const std::string_view rest = sep == std::string_view::npos
        ? std::string_view{} : trim(trimmed.substr(sep));

    const auto it = std::find_if(COMMAND_INFOS.begin(), COMMAND_INFOS.end(),
        [verb](const CommandInfo &info) noexcept { return info.name == verb; });
    std::optional<ParsedCommand> result = std::nullopt;
    if (it != COMMAND_INFOS.end() && it->hasArgument == !rest.empty())
        result = ParsedCommand{
            static_cast<CommandKind>(it - COMMAND_INFOS.begin()),
            std::string(rest)};
    return result;
}
