/*
** EPITECH PROJECT, 2026
** PlayerPanel.cpp
** File description:
** raygui player profile panel implementation
*/

#include "gui/ui/PlayerPanel.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>

#include "raygui.h"

namespace zappy::gui::ui {

namespace {

constexpr float PANEL_W = 300.0f;
constexpr float PANEL_H = 600.0f;

constexpr Color PANEL_TEXT = {235, 235, 240, 255};

constexpr std::array<const char *, board_data::RESOURCE_COUNT> RES_NAMES = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"
};

bool is_number(const char *s)
{
    if (s == nullptr || *s == '\0')
        return false;
    for (; *s != '\0'; ++s)
        if (std::isdigit(static_cast<unsigned char>(*s)) == 0)
            return false;
    return true;
}

const char *orient_str(board_data::Orientation o)
{
    switch (o) {
        case board_data::Orientation::NORTH: return "N";
        case board_data::Orientation::EAST: return "E";
        case board_data::Orientation::SOUTH: return "S";
        default: return "W";
    }
}

} // namespace

PlayerPanel::PlayerPanel(board_data::BoardData &board)
    : _board(board)
{
}

bool PlayerPanel::blocks_point(Vector2 p) const
{
    return _visible && CheckCollisionPointRec(p, PANEL_RECT);
}

void PlayerPanel::handle_toggle()
{
    if (IsKeyPressed(KEY_F3))
        _visible = !_visible;
}

std::vector<int> PlayerPanel::sorted_ids() const
{
    std::vector<int> ids;

    ids.reserve(_board.players().size());
    for (const auto &[id, player] : _board.players()) {
        (void)player;
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

void PlayerPanel::ensure_selection()
{
    const std::vector<int> ids = sorted_ids();
    const int id = _board.selected_player_id();

    if (ids.empty()) {
        if (id >= 0)
            _board.clear_selection();
        return;
    }
    if (id < 0 || _board.players().find(id) == _board.players().end())
        _board.select_player_local(ids.front());
}

void PlayerPanel::select_at(const std::vector<int> &ids, int index)
{
    const int n = static_cast<int>(ids.size());

    if (n == 0)
        return;
    const int i = ((index % n) + n) % n;
    if (ids[static_cast<std::size_t>(i)] != _board.selected_player_id())
        _board.select_player_local(ids[static_cast<std::size_t>(i)]);
}

void PlayerPanel::draw_nav(const Rectangle &box)
{
    const std::vector<int> ids = sorted_ids();

    if (ids.empty())
        return;
    const int id = _board.selected_player_id();
    const auto it = std::find(ids.begin(), ids.end(), id);
    const int idx = it == ids.end() ? 0
        : static_cast<int>(std::distance(ids.begin(), it));
    const float y = box.y + 30;

    if (GuiButton({box.x + 12, y, 40, 26}, "<"))
        select_at(ids, idx - 1);
    if (GuiButton({box.x + box.width - 52, y, 40, 26}, ">"))
        select_at(ids, idx + 1);
    GuiLabel({box.x + 60, y, box.width - 120, 26},
        TextFormat("#%d  (%d/%d)", id, idx + 1, static_cast<int>(ids.size())));
}

void PlayerPanel::draw_empty(const Rectangle &box)
{
    DrawText("No players", static_cast<int>(box.x) + 12,
        static_cast<int>(box.y) + 40, 22, PANEL_TEXT);
}

void PlayerPanel::draw()
{
    handle_toggle();
    if (!_visible)
        return;
    ensure_selection();
    const Rectangle box = PANEL_RECT;
    if (GuiWindowBox(box, "Player")) {
        _visible = false;
        return;
    }
    const int id = _board.selected_player_id();
    if (id < 0) {
        draw_empty(box);
        return;
    }
    draw_nav(box);
    if (_board.profile_disabled())
        draw_disabled(box);
    else
        draw_profile(box, id);
}

void PlayerPanel::draw_header(int x, int y, int id,
    const board_data::PlayerData &p)
{
    DrawText(TextFormat("id: #%d", id), x, y, 22, PANEL_TEXT);
    DrawText(TextFormat("team: %s", p.team.c_str()), x, y + 24, 22, PANEL_TEXT);
    DrawText(TextFormat("pos: %d,%d", p.x, p.y), x, y + 48, 22, PANEL_TEXT);
    DrawText(TextFormat("orient: %s", orient_str(p.orient)),
        x, y + 72, 22, PANEL_TEXT);
    DrawText(TextFormat("level: %d", p.level), x, y + 96, 22, PANEL_TEXT);
    DrawText(TextFormat("is_alive: %s", (p.alive) ? "True" : "False"), x, y + 120, 22, PANEL_TEXT);
}

void PlayerPanel::draw_inventory(int x, int y,
    const board_data::PlayerData &p)
{
    DrawText("inventory:", x, y, 22, PANEL_TEXT);
    for (std::size_t i = 0; i < board_data::RESOURCE_COUNT; ++i)
        DrawText(TextFormat("%s: %d", RES_NAMES[i], p.inventory[i]),
            x, y + 26 + static_cast<int>(i) * 20, 22, PANEL_TEXT);
}

void PlayerPanel::draw_buttons(const Rectangle &box, int id)
{
    const float by = box.y + box.height - 32;

    if (GuiButton({box.x + 12, by, 110, 24}, "Refresh"))
        _board.set_selected_player(id);
    if (GuiButton({box.x + box.width - 122, by, 110, 24}, "Close"))
        _visible = false;
}

void PlayerPanel::submit_tp(int id)
{
    if (!is_number(_tp_x) || !is_number(_tp_y))
        return;
    _board.queue_admin_command(std::string("adm_player_tp #")
        + std::to_string(id) + " " + _tp_x + " " + _tp_y + "\n");
}

void PlayerPanel::submit_level(int id)
{
    if (!is_number(_lvl))
        return;
    _board.queue_admin_command(std::string("adm_player_level #")
        + std::to_string(id) + " " + _lvl + "\n");
}

void PlayerPanel::draw_tp_row(const Rectangle &box, float y, int id)
{
    const float x = box.x + 12;

    if (GuiTextBox({x, y, 50, 24}, _tp_x, sizeof(_tp_x), _tp_x_edit))
        _tp_x_edit = !_tp_x_edit;
    if (GuiTextBox({x + 56, y, 50, 24}, _tp_y, sizeof(_tp_y), _tp_y_edit))
        _tp_y_edit = !_tp_y_edit;
    if (GuiButton({x + 112, y, 120, 24}, "Teleport"))
        submit_tp(id);
}

void PlayerPanel::draw_level_row(const Rectangle &box, float y, int id)
{
    const float x = box.x + 12;

    if (GuiTextBox({x, y, 50, 24}, _lvl, sizeof(_lvl), _lvl_edit))
        _lvl_edit = !_lvl_edit;
    if (GuiButton({x + 56, y, 176, 24}, "Set level"))
        submit_level(id);
}

void PlayerPanel::draw_incant_row(const Rectangle &box, float y, int id)
{
    const float x = box.x + 12;

    if (GuiButton({x, y, 110, 24}, "Incant"))
        _board.queue_admin_command(std::string("adm_player_incant #")
            + std::to_string(id) + "\n");
    if (GuiButton({x + 122, y, 110, 24}, "Stop incant"))
        _board.queue_admin_command(std::string("adm_player_stop_incant #")
            + std::to_string(id) + "\n");
}

void PlayerPanel::draw_admin_actions(const Rectangle &box, int id)
{
    const float x = box.x + 12;
    const float y = box.y + 379;

    GuiLabel({x, y, 200, 20}, "Admin actions:");
    if (GuiButton({x, y + 20, 110, 24}, "Kill"))
        _board.queue_admin_command(std::string("adm_player_kill #")
            + std::to_string(id) + "\n");
    draw_tp_row(box, y + 50, id);
    draw_level_row(box, y + 82, id);
    draw_incant_row(box, y + 114, id);
}

void PlayerPanel::draw_profile(const Rectangle &box, int id)
{
    auto it = _board.players().find(id);

    if (it == _board.players().end())
        return;
    const int x = static_cast<int>(box.x) + 12;
    const int y = static_cast<int>(box.y) + 64;
    draw_header(x, y, id, it->second);
    draw_inventory(x, y + LINE_BREAK_PADDING * 7, it->second);
    if (_board.is_admin())
        draw_admin_actions(box, id);
    draw_buttons(box, id);
}

bool PlayerPanel::capturing_keyboard() const noexcept
{
    return _board.selected_player_id() >= 0
        && (_tp_x_edit || _tp_y_edit || _lvl_edit);
}

void PlayerPanel::draw_disabled(const Rectangle &box)
{
    const int x = static_cast<int>(box.x) + 12;
    const int y = static_cast<int>(box.y) + 64;

    DrawText("profile flag", x, y, 22, MAROON);
    DrawText("disabled on server", x, y + 26, 22, MAROON);
    if (GuiButton({box.x + 12, box.y + box.height - 32, 110, 24}, "Close"))
        _visible = false;
}

} // namespace zappy::gui::ui
