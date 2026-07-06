/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Static metadata for every GUI command (name, argument count)
*/

#ifndef PROTOCOL_GUI_GUICOMMANDINFO_HPP_
    #define PROTOCOL_GUI_GUICOMMANDINFO_HPP_

    #include <array>
    #include <cstddef>
    #include <string_view>

    #include "protocol/gui/GuiCommandKind.hpp"

namespace zappy::protocol::gui {

/**
 * @brief Static description of a single GUI command.
 */
struct GuiCommandInfo {
    std::string_view name;
    int intArgCount;
    bool acceptsRawArgument;

    /**
     * @brief Table entry for @p kind.
     *
     * @param kind command to describe
     * @return const GuiCommandInfo& the matching entry
     */
    [[nodiscard]] static constexpr const GuiCommandInfo &guiCommandInfoFor(
        GuiCommandKind kind) noexcept;
};

inline constexpr std::array<GuiCommandInfo,
    GUI_COMMAND_KIND_COUNT> GUI_COMMAND_INFOS = {{
    {"msz", 0, false},
    {"bct", 2, false},
    {"mct", 0, false},
    {"tna", 0, false},
    {"ppo", 1, false},
    {"plv", 1, false},
    {"pin", 1, false},
    {"sgt", 0, false},
    {"sst", 1, false},
    {"ppf", 1, false},
    {"admin", 0, true},
    {"adm_flag_enable", 0, true},
    {"adm_flag_disable", 0, true},
    {"adm_flag_list", 0, false},
    {"adm_event_trigger", 0, true},
    {"adm_tile_set", 3, true},
    {"adm_tile_add", 3, true},
    {"adm_player_kill", 1, false},
    {"adm_player_tp", 3, false},
    {"adm_player_level", 2, false},
    {"adm_player_incant", 1, false},
    {"adm_player_stop_incant", 1, false}
}};

constexpr const GuiCommandInfo &GuiCommandInfo::guiCommandInfoFor(
    GuiCommandKind kind) noexcept
{
    return GUI_COMMAND_INFOS[static_cast<std::size_t>(kind)];
}

} // namespace zappy::protocol::gui

#endif /* !PROTOCOL_GUI_GUICOMMANDINFO_HPP_ */
