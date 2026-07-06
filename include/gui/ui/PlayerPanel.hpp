/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** raygui panel showing the extended profile of a selected player
*/

#ifndef GUI_UI_PLAYERPANEL_HPP_
    #define GUI_UI_PLAYERPANEL_HPP_
    #define LINE_BREAK_PADDING 24

    #include <raylib.h>

    #include <vector>

    #include "gui/board_data/BoardData.hpp"

namespace zappy::gui::ui {

#define PANEL_RECT {static_cast<float>(GetScreenWidth()) - PANEL_W - 20.0f,\
    80.0f, PANEL_W, PANEL_H}

class PlayerPanel {
public:
    explicit PlayerPanel(board_data::BoardData &board);

    void draw();
    [[nodiscard]] bool blocks_point(Vector2 p) const;
    [[nodiscard]] bool capturing_keyboard() const noexcept;

private:
    [[nodiscard]] Rectangle panel_rect() const;
    void handle_toggle();
    void ensure_selection();
    void draw_nav(const Rectangle &box);
    void draw_empty(const Rectangle &box);
    void select_at(const std::vector<int> &ids, int index);
    [[nodiscard]] std::vector<int> sorted_ids() const;
    void draw_profile(const Rectangle &box, int id);
    void draw_disabled(const Rectangle &box);
    void draw_header(int x, int y, int id, const board_data::PlayerData &p);
    void draw_inventory(int x, int y, const board_data::PlayerData &p);
    void draw_buttons(const Rectangle &box, int id);
    void draw_admin_actions(const Rectangle &box, int id);
    void draw_tp_row(const Rectangle &box, float y, int id);
    void draw_level_row(const Rectangle &box, float y, int id);
    void draw_incant_row(const Rectangle &box, float y, int id);
    void submit_tp(int id);
    void submit_level(int id);

    board_data::BoardData &_board;
    char _tp_x[8] = {0};
    char _tp_y[8] = {0};
    char _lvl[4] = {0};
    bool _tp_x_edit = false;
    bool _tp_y_edit = false;
    bool _lvl_edit = false;
    bool _visible = true;
};

} // namespace zappy::gui::ui

#endif /* !GUI_UI_PLAYERPANEL_HPP_ */
