/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** A successfully parsed AI command line
*/

#ifndef PROTOCOL_AI_PARSEDCOMMAND_HPP_
    #define PROTOCOL_AI_PARSEDCOMMAND_HPP_

    #include <string>

    #include "protocol/ai/CommandKind.hpp"

namespace zappy::protocol::ai {

/**
 * @brief A parsed command: its kind and its raw argument (empty if none).
 */
struct ParsedCommand {
    CommandKind kind;
    std::string argument;
};

} // namespace zappy::protocol::ai

#endif /* !PROTOCOL_AI_PARSEDCOMMAND_HPP_ */
