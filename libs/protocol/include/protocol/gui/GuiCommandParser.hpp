/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Stateless parser turning a raw GUI line into a ParsedGuiCommand
*/

#ifndef PROTOCOL_GUI_GUICOMMANDPARSER_HPP_
    #define PROTOCOL_GUI_GUICOMMANDPARSER_HPP_

    #include <string_view>
    #include <variant>
    #include <vector>

    #include "protocol/gui/GuiCommandInfo.hpp"
    #include "protocol/gui/ParsedGuiCommand.hpp"

namespace zappy::protocol::gui {

/**
 * @brief Parses a GUI command line; pure, stateless,
 * no allocation on failure.
 */
class GuiCommandParser {
public:
    /**
     * @brief Parse @p line into a ParsedGuiCommand or a GuiParseError.
     *
     * Trims surrounding whitespace, splits the verb from its arguments and
     * validates the arguments against the command metadata.
     *
     * @param line raw command line (no trailing newline required)
     * @return std::variant<ParsedGuiCommand, GuiParseError>
     */
    [[nodiscard]] static std::variant<ParsedGuiCommand,
        GuiParseError> parse(std::string_view line);

private:
    /**
     * @brief Strip leading and trailing spaces, tabs and carriage returns.
     *
     * @param s raw view
     * @return std::string_view trimmed view into @p s
     */
    [[nodiscard]] static std::string_view trim(std::string_view s) noexcept;

    /**
     * @brief Split the arguments block into individual tokens.
     *
     * @param rest the trimmed argument block string
     * @return std::vector<std::string_view> argument tokens
     */
    [[nodiscard]] static std::vector<std::string_view> tokenize(
        std::string_view rest);

    /**
     * @brief Parse the first @p count tokens into non-negative integers.
     *
     * @param tokens individual tokens
     * @param count number of leading tokens to convert
     * @param args target vector to populate
     * @return bool true if every converted token was a non-negative integer
     */
    [[nodiscard]] static bool convertArgs(
        const std::vector<std::string_view> &tokens, std::size_t count,
        std::vector<int> &args) noexcept;

    /**
     * @brief Trimmed remainder of @p rest after the first @p count tokens.
     *
     * @param rest the trimmed argument block
     * @param tokens the tokens of @p rest
     * @param count number of leading tokens consumed as integers
     * @return std::string_view the raw remainder, empty if none
     */
    [[nodiscard]] static std::string_view rawAfter(std::string_view rest,
        const std::vector<std::string_view> &tokens,
        std::size_t count) noexcept;

    /**
     * @brief Build a command from its leading integers and optional raw tail.
     *
     * @param info matched command metadata
     * @param kind the command kind
     * @param rest the trimmed argument block
     * @param tokens the tokens of @p rest
     * @return std::variant<ParsedGuiCommand, GuiParseError>
     */
    [[nodiscard]] static std::variant<ParsedGuiCommand,
        GuiParseError> buildMixed(const GuiCommandInfo &info,
            GuiCommandKind kind, std::string_view rest,
            const std::vector<std::string_view> &tokens);

    /**
     * @brief Resolve a matched command into a parse result.
     *
     * Tokenizes the argument block then validates the leading integers and the
     * optional trailing raw argument against the command metadata.
     *
     * @param info matched command metadata
     * @param kind the command kind
     * @param rest the trimmed argument block
     * @return std::variant<ParsedGuiCommand, GuiParseError>
     */
    [[nodiscard]] static std::variant<ParsedGuiCommand,
        GuiParseError> resolve(const GuiCommandInfo &info,
            GuiCommandKind kind, std::string_view rest);
};

} // namespace zappy::protocol::gui

#endif /* !PROTOCOL_GUI_GUICOMMANDPARSER_HPP_ */
