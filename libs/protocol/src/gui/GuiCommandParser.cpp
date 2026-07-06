/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GuiCommandParser implementation
*/

#include <algorithm>
#include <charconv>
#include <string>
#include <system_error>

#include "protocol/gui/GuiCommandInfo.hpp"
#include "protocol/gui/GuiCommandParser.hpp"

/**
 * @brief Trim leading and trailing spaces, tabs and carriage returns.
 *
 * @param s raw view
 * @return std::string_view trimmed view
 */
std::string_view zappy::protocol::gui::GuiCommandParser::trim(
    std::string_view s) noexcept
{
    const std::size_t first = s.find_first_not_of(" \t\r");
    const std::size_t last = s.find_last_not_of(" \t\r");
    return first == std::string_view::npos
        ? std::string_view{} : s.substr(first, last - first + 1);
}

/**
 * @brief Split the arguments block into individual tokens.
 *
 * @param rest the trimmed argument block string
 * @return std::vector<std::string_view> argument tokens
 */
std::vector<std::string_view> zappy::protocol::gui::GuiCommandParser::tokenize(
    std::string_view rest)
{
    std::vector<std::string_view> tokens;
    std::size_t start = rest.find_first_not_of(" \t\r");
    while (start != std::string_view::npos) {
        const std::size_t end = rest.find_first_of(" \t\r", start);
        tokens.push_back(end == std::string_view::npos
            ? rest.substr(start) : rest.substr(start, end - start));
        start = end == std::string_view::npos
            ? std::string_view::npos : rest.find_first_not_of(" \t\r", end);
    }
    return tokens;
}

/**
 * @brief Parse the first @p count tokens into non-negative integers.
 *
 * @param tokens individual tokens
 * @param count number of leading tokens to convert
 * @param args target vector to populate
 * @return bool true if every converted token was a non-negative integer
 */
bool zappy::protocol::gui::GuiCommandParser::convertArgs(
    const std::vector<std::string_view> &tokens, std::size_t count,
    std::vector<int> &args) noexcept
{
    bool ok = true;
    for (std::size_t i = 0; i < count && i < tokens.size(); ++i) {
        int val = 0;
        const std::string_view tok = tokens[i];
        const std::string_view num = (!tok.empty() && tok.front() == '#')
            ? tok.substr(1) : tok;
        const auto [ptr, ec] = std::from_chars(
            num.data(), num.data() + num.size(), val);
        if (ec != std::errc{} || ptr != num.data() + num.size() || val < 0)
            ok = false;
        else
            args.push_back(val);
    }
    return ok;
}

/**
 * @brief Trimmed remainder of @p rest after the first @p count tokens.
 *
 * @param rest the trimmed argument block
 * @param tokens the tokens of @p rest
 * @param count number of leading tokens consumed as integers
 * @return std::string_view the raw remainder, empty if none
 */
std::string_view zappy::protocol::gui::GuiCommandParser::rawAfter(
    std::string_view rest, const std::vector<std::string_view> &tokens,
    std::size_t count) noexcept
{
    std::string_view result;

    if (count < tokens.size()) {
        const std::size_t off = static_cast<std::size_t>(
            tokens[count].data() - rest.data());
        result = trim(rest.substr(off));
    }
    return result;
}

/**
 * @brief Build a command from its leading integers and optional raw tail.
 *
 * @param info matched command metadata
 * @param kind the command kind
 * @param rest the trimmed argument block
 * @param tokens the tokens of @p rest
 * @return std::variant<ParsedGuiCommand, GuiParseError>
 */
std::variant<zappy::protocol::gui::ParsedGuiCommand,
    zappy::protocol::gui::GuiParseError>
zappy::protocol::gui::GuiCommandParser::buildMixed(
    const GuiCommandInfo &info, GuiCommandKind kind, std::string_view rest,
    const std::vector<std::string_view> &tokens)
{
    std::variant<ParsedGuiCommand, GuiParseError> result =
        GuiParseError::BAD_PARAMETER;
    const std::size_t need = static_cast<std::size_t>(info.intArgCount);
    const bool countOk = info.acceptsRawArgument
        ? tokens.size() >= need : tokens.size() == need;
    std::vector<int> args;
    std::string_view raw;

    if (countOk && convertArgs(tokens, need, args)) {
        raw = rawAfter(rest, tokens, need);
        if (!info.acceptsRawArgument)
            result = ParsedGuiCommand{kind, std::move(args), std::string{}};
        else if (!raw.empty())
            result = ParsedGuiCommand{kind, std::move(args), std::string(raw)};
    }
    return result;
}

/**
 * @brief Resolve a matched command into a parse result.
 *
 * @param info matched command metadata
 * @param kind the command kind
 * @param rest the trimmed argument block
 * @return std::variant<ParsedGuiCommand, GuiParseError>
 */
std::variant<zappy::protocol::gui::ParsedGuiCommand,
    zappy::protocol::gui::GuiParseError>
zappy::protocol::gui::GuiCommandParser::resolve(
    const GuiCommandInfo &info, GuiCommandKind kind, std::string_view rest)
{
    return buildMixed(info, kind, rest, tokenize(rest));
}

/**
 * @brief Parse a GUI command line.
 *
 * @param line raw command line
 * @return std::variant<ParsedGuiCommand, GuiParseError>
 */
std::variant<zappy::protocol::gui::ParsedGuiCommand,
    zappy::protocol::gui::GuiParseError>
zappy::protocol::gui::GuiCommandParser::parse(std::string_view line)
{
    const std::string_view trimmed = trim(line);
    std::variant<ParsedGuiCommand, GuiParseError> result =
        GuiParseError::UNKNOWN_COMMAND;
    const std::size_t sep = trimmed.find_first_of(" \t\r");
    const std::string_view verb = trimmed.substr(0, sep);
    const std::string_view rest = sep == std::string_view::npos
        ? std::string_view{} : trim(trimmed.substr(sep));
    const auto it = std::find_if(GUI_COMMAND_INFOS.begin(),
        GUI_COMMAND_INFOS.end(), [verb](const GuiCommandInfo &info) noexcept {
            return info.name == verb;
        });

    if (!trimmed.empty() && it != GUI_COMMAND_INFOS.end())
        result = resolve(*it,
            static_cast<GuiCommandKind>(it - GUI_COMMAND_INFOS.begin()), rest);
    return result;
}
