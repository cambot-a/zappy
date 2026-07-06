/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Closed set of AI commands understood by the server
*/

#ifndef PROTOCOL_AI_COMMANDKIND_HPP_
    #define PROTOCOL_AI_COMMANDKIND_HPP_

    #include <cstddef>

namespace zappy::protocol::ai {

/**
 * @brief Every AI command, ordered to index the COMMAND_INFOS table.
 */
enum class CommandKind {
    FORWARD,
    RIGHT,
    LEFT,
    LOOK,
    INVENTORY,
    BROADCAST,
    CONNECT_NBR,
    FORK,
    EJECT,
    TAKE,
    SET,
    INCANTATION
};

constexpr std::size_t COMMAND_KIND_COUNT = 12;

} // namespace zappy::protocol::ai

#endif /* !PROTOCOL_AI_COMMANDKIND_HPP_ */
