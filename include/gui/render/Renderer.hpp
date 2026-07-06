/*
** EPITECH PROJECT, 2026
** Renderer.hpp
** File description:
** Raylib window owner driven by BoardData
*/

#ifndef GUI_RENDER_RENDERER_HPP_
    #define GUI_RENDER_RENDERER_HPP_

    #include "gui/board_data/BoardData.hpp"
    #include "gui/camera.hpp"
    #include "gui/render/Scene.hpp"
    #include "gui/ui/AdminPanel.hpp"
    #include "gui/ui/Chat.hpp"
    #include "gui/ui/PlayerPanel.hpp"
    #include "gui/window.hpp"
    #include "skybox.h"

namespace zappy::gui::render {

class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    [[nodiscard]] bool should_close();
    void draw(board_data::BoardData &board, ui::AdminPanel &admin,
        ui::PlayerPanel &player);

private:
    void setup_shader();
    void update_light();
    void overlay(const board_data::BoardData &board);
    void storm_overlay(const board_data::BoardData &board);
    void render_dist_hud();
    void drain_events(board_data::BoardData &board);
    void update_cursor();
    void handle_pick(board_data::BoardData &board, ui::AdminPanel &admin,
        ui::PlayerPanel &player);
    void trigger_event(board_data::BoardData &board, Vector2 mouse);
    void draw_crosshair();
    void update_skybox();
    void draw_skybox();

    window _window;
    camera _camera;
    Scene _scene;
    ui::Chat _chat;
    Shader _shader{};
    Skyboxlib _skybox;
};

} // namespace zappy::gui::render

#endif /* GUI_RENDER_RENDERER_HPP_ */
