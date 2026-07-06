/*
** EPITECH PROJECT, 2026
** GraphicalEvent.hpp
** File description:
** GraphicalEvent
*/

#ifndef GRAPHICALEVENT_HPP_
    #define GRAPHICALEVENT_HPP_
    #include <string>

namespace zappy::gui::board_data {

enum class GraphicalEventKind {
    BROADCAST,
    INCANTATION_START,
    INCANTATION_END,
    EJECT,
    FORK
};

struct GraphicalEvent {
    GraphicalEventKind kind = GraphicalEventKind::BROADCAST;
    int player_id = -1;
    int x = 0;
    int y = 0;
    int level = 0;
    bool success = false;
    std::string text;
    float lifetime = 0.0f;
    bool alive = true;
};

} // namespace zappy::gui::board_data

#endif /* GRAPHICALEVENT_HPP_ */
