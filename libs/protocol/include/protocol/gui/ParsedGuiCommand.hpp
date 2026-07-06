/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** A successfully parsed GUI command line
*/

#ifndef PROTOCOL_GUI_PARSEDGUICOMMAND_HPP_
    #define PROTOCOL_GUI_PARSEDGUICOMMAND_HPP_

    #include <string>
    #include <vector>

    #include "protocol/gui/GuiCommandKind.hpp"

namespace zappy::protocol::gui {

/**
 * @brief A parsed GUI command: its kind, integer arguments and raw argument.
 *
 * @c rawArgument holds the unparsed string payload of commands flagged with
 * @c acceptsRawArgument (e.g. the admin password); empty for all others.
 */
struct ParsedGuiCommand {
    GuiCommandKind kind;
    std::vector<int> arguments;
    std::string rawArgument;
};

/**
 * @brief GUI protocol parsing errors.
 */
enum class GuiParseError {
    UNKNOWN_COMMAND,
    BAD_PARAMETER
};

} // namespace zappy::protocol::gui

#endif /* !PROTOCOL_GUI_PARSEDGUICOMMAND_HPP_ */
