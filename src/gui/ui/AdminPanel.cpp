/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** raygui admin authentication and feature-flag panel implementation
*/

#include "gui/ui/AdminPanel.hpp"

#include <raylib.h>

#include <cstdlib>

#include "raygui.h"

namespace zappy::gui::ui {

namespace {

constexpr float PANEL_X = 20.0f;
constexpr float PANEL_Y = 80.0f;
constexpr float PANEL_W = 280.0f;
constexpr float PANEL_H = 400.0f;
constexpr double STATUS_DURATION = 2.0;

} // namespace

AdminPanel::AdminPanel(net::ServerLink &link, board_data::BoardData &board)
    : _link(link), _board(board)
{
}

void AdminPanel::handle_toggle()
{
    if (IsKeyPressed(KEY_F2))
        _visible = !_visible;
}

bool AdminPanel::blocks_point(Vector2 p) const
{
    return _visible
        && CheckCollisionPointRec(p, {PANEL_X, PANEL_Y, PANEL_W, PANEL_H});
}

void AdminPanel::sync_status()
{
    const unsigned seq = _board.admin_status_seq();

    if (seq == _last_seq)
        return;
    _last_seq = seq;
    _status_text = _board.admin_status();
    _status_until = GetTime() + STATUS_DURATION;
}

void AdminPanel::draw()
{
    handle_toggle();
    sync_status();
    if (!_visible)
        return;
    draw_window();
    if (_board.is_admin())
        draw_flags();
    else
        draw_login();
    draw_status();
}

void AdminPanel::draw_window()
{
    if (GuiWindowBox({PANEL_X, PANEL_Y, PANEL_W, PANEL_H}, "Admin"))
        _visible = false;
}

void AdminPanel::submit_login()
{
    _board.mark_admin_pending();
    _link.send(std::string("admin ") + _password + "\n");
    _password_edit = false;
}

void AdminPanel::draw_login()
{
    const Rectangle box{PANEL_X + 15, PANEL_Y + 50, PANEL_W - 30, 28};
    const Rectangle btn{PANEL_X + 15, PANEL_Y + 90, PANEL_W - 30, 30};

    GuiLabel({PANEL_X + 15, PANEL_Y + 32, PANEL_W - 30, 20}, "Password:");
    if (GuiTextBox(box, _password, sizeof(_password), _password_edit))
        _password_edit = !_password_edit;
    if (GuiButton(btn, "Login"))
        submit_login();
}

void AdminPanel::draw_flag_toggle(const std::string &name, bool on, float y)
{
    if (!GuiButton({PANEL_X + 150, y - 4, 110, 26}, on ? "Disable" : "Enable"))
        return;
    _link.send((on ? "adm_flag_disable " : "adm_flag_enable ") + name + "\n");
    _link.send("adm_flag_list\n");
}

void AdminPanel::draw_flag_row(const std::string &name, bool on, float y)
{
    GuiLabel({PANEL_X + 20, y, 80, 22}, name.c_str());
    DrawText(on ? "on" : "off", static_cast<int>(PANEL_X + 105),
        static_cast<int>(y), 22, on ? GREEN : GRAY);
    draw_flag_toggle(name, on, y);
}

bool AdminPanel::events_on() const
{
    const auto &flags = _board.feature_flags();
    const auto it = flags.find("events");

    return it != flags.end() && it->second;
}

int AdminPanel::radius_value() const
{
    const int r = std::atoi(_radius_buf);

    return r > 0 ? r : 3;
}

void AdminPanel::arm(const char *name)
{
    _board.arm_event(name, radius_value());
}

void AdminPanel::draw_event_triggers(float y)
{
    GuiLabel({PANEL_X + 15, y, 130, 22}, "Trigger (R=)");
    if (GuiTextBox({PANEL_X + 150, y - 2, 50, 26}, _radius_buf,
        sizeof(_radius_buf), _radius_edit))
        _radius_edit = !_radius_edit;
    if (!events_on())
        GuiSetState(STATE_DISABLED);
    if (GuiButton({PANEL_X + 15, y + 24, 78, 30}, "Storm"))
        arm("storm");
    if (GuiButton({PANEL_X + 101, y + 24, 78, 30}, "Meteor"))
        arm("meteor");
    if (GuiButton({PANEL_X + 187, y + 24, 78, 30}, "Flood"))
        arm("flood");
    GuiSetState(STATE_NORMAL);
}

void AdminPanel::draw_flags()
{
    float y = PANEL_Y + 34;

    GuiLabel({PANEL_X + 15, PANEL_Y + 30, PANEL_W - 30, 22}, "Feature flags:");
    y += 24;
    for (const auto &[name, on] : _board.feature_flags()) {
        draw_flag_row(name, on, y);
        y += 30;
    }
    draw_event_triggers(y + 8);
    if (GuiButton({PANEL_X + 15, PANEL_Y + PANEL_H - 40, PANEL_W - 30, 28},
        "Refresh"))
        _link.send("adm_flag_list\n");
}

void AdminPanel::draw_status()
{
    if (_status_text.empty() || GetTime() > _status_until)
        return;
    DrawText(_status_text.c_str(), static_cast<int>(PANEL_X + 15),
        static_cast<int>(PANEL_Y + PANEL_H - 68), 20, MAROON);
}

} // namespace zappy::gui::ui
