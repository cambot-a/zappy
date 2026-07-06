/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Static metadata for every AI command (name, duration, argument)
*/

#ifndef PROTOCOL_AI_COMMANDINFO_HPP_
    #define PROTOCOL_AI_COMMANDINFO_HPP_

    #include <array>
    #include <chrono>
    #include <cstddef>
    #include <string_view>

    #include "protocol/ai/CommandKind.hpp"

namespace zappy::protocol::ai {

using Duration = std::chrono::milliseconds;

/**
 * @brief Static description of a single AI command.
 */
struct CommandInfo {
    std::string_view name;
    int durationTimeUnits;
    bool hasArgument;

    /**
     * @brief Table entry for @p kind.
     *
     * @param kind command to describe
     * @return const CommandInfo& the matching entry
     */
    [[nodiscard]] static constexpr const CommandInfo &infoFor(
        CommandKind kind) noexcept;

    /**
     * @brief Execution time of @p kind at the given frequency.
     *
     * @param kind command to time
     * @param frequency reciprocal of the time unit (the f config value)
     * @return Duration the command duration, 0 for instantaneous commands
     */
    [[nodiscard]] static constexpr Duration duration(
        CommandKind kind, int frequency) noexcept;
};

inline constexpr std::array<CommandInfo, COMMAND_KIND_COUNT> COMMAND_INFOS = {{
    {"Forward", 7, false},
    {"Right", 7, false},
    {"Left", 7, false},
    {"Look", 7, false},
    {"Inventory", 1, false},
    {"Broadcast", 7, true},
    {"Connect_nbr", 0, false},
    {"Fork", 42, false},
    {"Eject", 7, false},
    {"Take", 7, true},
    {"Set", 7, true},
    {"Incantation", 300, false}
}};

constexpr const CommandInfo &CommandInfo::infoFor(CommandKind kind) noexcept
{
    return COMMAND_INFOS[static_cast<std::size_t>(kind)];
}

constexpr Duration CommandInfo::duration(
    CommandKind kind, int frequency) noexcept
{
    const CommandInfo &info = infoFor(kind);
    return Duration(frequency > 0
        ? info.durationTimeUnits * 1000 / frequency
        : info.durationTimeUnits * 1000);
}

} // namespace zappy::protocol::ai

#endif /* !PROTOCOL_AI_COMMANDINFO_HPP_ */
