/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Closed set of GUI commands understood by the server
*/

#ifndef PROTOCOL_GUI_GUICOMMANDKIND_HPP_
    #define PROTOCOL_GUI_GUICOMMANDKIND_HPP_

    #include <cstddef>

namespace zappy::protocol::gui {

/**
 * @brief Every GUI command, ordered to index the GUI_COMMAND_INFOS table.
 */
enum class GuiCommandKind {
    MSZ,
    BCT,
    MCT,
    TNA,
    PPO,
    PLV,
    PIN,
    SGT,
    SST,
    PPF,
    ADMIN,
    ADM_FLAG_ENABLE,
    ADM_FLAG_DISABLE,
    ADM_FLAG_LIST,
    ADM_EVENT_TRIGGER,
    ADM_TILE_SET,
    ADM_TILE_ADD,
    ADM_PLAYER_KILL,
    ADM_PLAYER_TP,
    ADM_PLAYER_LEVEL,
    ADM_PLAYER_INCANT,
    ADM_PLAYER_STOP_INCANT
};

constexpr std::size_t GUI_COMMAND_KIND_COUNT = 22;

} // namespace zappy::protocol::gui

#endif /* !PROTOCOL_GUI_GUICOMMANDKIND_HPP_ */
