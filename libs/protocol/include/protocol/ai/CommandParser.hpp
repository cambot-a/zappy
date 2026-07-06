/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Stateless parser turning a raw line into a ParsedCommand
*/

#ifndef PROTOCOL_AI_COMMANDPARSER_HPP_
    #define PROTOCOL_AI_COMMANDPARSER_HPP_

    #include <optional>
    #include <string_view>

    #include "protocol/ai/ParsedCommand.hpp"

namespace zappy::protocol::ai {

/**
 * @brief Parses an AI command line; pure, stateless, no allocation on failure.
 */
class CommandParser {
public:
    /**
     * @brief Parse @p line into a ParsedCommand.
     *
     * Trims surrounding whitespace, splits the verb from its argument and
     * validates the argument against the command metadata.
     *
     * @param line raw command line (no trailing newline required)
     * @return std::optional<ParsedCommand> nullopt if unknown or malformed
     */
    [[nodiscard]] static std::optional<ParsedCommand> parse(
        std::string_view line);

private:
    /**
     * @brief Strip leading and trailing spaces, tabs and carriage returns.
     *
     * @param s raw view
     * @return std::string_view trimmed view into @p s
     */
    [[nodiscard]] static std::string_view trim(std::string_view s) noexcept;
};

} // namespace zappy::protocol::ai

#endif /* !PROTOCOL_AI_COMMANDPARSER_HPP_ */
