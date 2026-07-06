/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Fading broadcast chat overlay
*/

#ifndef GUI_UI_CHAT_HPP_
    #define GUI_UI_CHAT_HPP_

    #include <deque>
    #include <string>

namespace zappy::gui::ui {

class Chat {
public:
    // Add a broadcast line from player @p player_id.
    void push(int player_id, const std::string &text);
    // Draw the recent lines bottom-left, fading the oldest out.
    void draw();

private:
    struct Line {
        std::string text;
        double time = 0.0;
    };

    std::deque<Line> _lines;
};

} // namespace zappy::gui::ui

#endif /* !GUI_UI_CHAT_HPP_ */
