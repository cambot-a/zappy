/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** raygui admin authentication and feature-flag panel
*/

#ifndef GUI_UI_ADMINPANEL_HPP_
    #define GUI_UI_ADMINPANEL_HPP_

    #include <raylib.h>

    #include <string>

    #include "gui/board_data/BoardData.hpp"
    #include "gui/net/ServerLink.hpp"

namespace zappy::gui::ui {

class AdminPanel {
public:
    AdminPanel(net::ServerLink &link, board_data::BoardData &board);

    void draw();
    [[nodiscard]] bool visible() const noexcept { return _visible; }
    [[nodiscard]] bool capturing_keyboard() const noexcept
        { return _visible && (_password_edit || _radius_edit); }
    [[nodiscard]] bool blocks_point(Vector2 p) const;

private:
    void handle_toggle();
    void sync_status();
    void draw_window();
    void draw_login();
    void draw_flags();
    void draw_flag_row(const std::string &name, bool on, float y);
    void draw_flag_toggle(const std::string &name, bool on, float y);
    void draw_event_triggers(float y);
    void arm(const char *name);
    [[nodiscard]] int radius_value() const;
    [[nodiscard]] bool events_on() const;
    void draw_status();
    void submit_login();

    net::ServerLink &_link;
    board_data::BoardData &_board;
    bool _visible = false;
    bool _password_edit = false;
    bool _radius_edit = false;
    char _password[64] = {0};
    char _radius_buf[4] = {0};
    unsigned _last_seq = 0;
    double _status_until = 0.0;
    std::string _status_text;
};

} // namespace zappy::gui::ui

#endif /* !GUI_UI_ADMINPANEL_HPP_ */
