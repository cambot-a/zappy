/*
** EPITECH PROJECT, 2026
** Renderer.cpp
** File description:
** Raylib window owner driven by BoardData
*/

#include "gui/render/Renderer.hpp"
#include "gui/utils.hpp"
#include <string>

#include "gui/draw.hpp"
#include "gui/trantor.hpp"
#include "gui/ui/Theme.hpp"
#include "raygui.h"

namespace zappy::gui::render {

namespace {

constexpr int WIN_WIDTH = 1280;
constexpr int WIN_HEIGHT = 720;

const char *dir_char(board_data::Orientation dir)
{
    switch (dir) {
        case board_data::Orientation::NORTH: return "N";
        case board_data::Orientation::EAST: return "E";
        case board_data::Orientation::SOUTH: return "S";
        default: return "W";
    }
}

int count_flooded(const board_data::BoardData &board)
{
    int count = 0;

    for (const auto &tile : board.tiles())
        if (tile.flooded)
            ++count;
    return count;
}

} // namespace

Renderer::Renderer()
    : _window(WIN_WIDTH, WIN_HEIGHT, "Zappy GUI"),
      _camera({10.0f, 10.0f, 10.0f})
{
    _camera.set_mode(CAMERA_FREE);
    SetTargetFPS(60);
    _window.Set_window_state(FLAG_WINDOW_RESIZABLE);
    ui::apply_modern_theme();
    setup_shader();
    _skybox = SkyboxLoad( "src/gui/assets/skybox/skyboxsea1.png", NULL,
        "src/gui/assets/skybox/gls330/skybox.vs", "src/gui/assets/skybox/gls330/skybox.fs",
        "src/gui/assets/skybox/gls330/cubemap.vs", "src/gui/assets/skybox/gls330/cubemap.fs");
}

Renderer::~Renderer()
{
    UnloadShader(_shader);
}

void Renderer::setup_shader()
{
    const float ambient[4] = {0.5f, 0.5f, 0.55f, 1.0f};
    const Vector3 dir = {-0.6f, -1.0f, -0.4f};

    _shader = LoadShader("src/gui/assets/lighting.vs",
        "src/gui/assets/lighting.fs");
    _shader.locs[SHADER_LOC_VECTOR_VIEW] =
        GetShaderLocation(_shader, "viewPos");
    SetShaderValue(_shader, GetShaderLocation(_shader, "ambient"),
        ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(_shader, GetShaderLocation(_shader, "lightDir"),
        &dir, SHADER_UNIFORM_VEC3);
    _scene.set_shader(_shader);
}

void Renderer::update_light()
{
    const Vector3 cam = _camera.cam.position;

    SetShaderValue(_shader, _shader.locs[SHADER_LOC_VECTOR_VIEW],
        &cam, SHADER_UNIFORM_VEC3);
}

bool Renderer::should_close()
{
    return _window.WindowIsCLose();
}

void Renderer::overlay(const board_data::BoardData &board)
{
    std::string hud_map_info = "map " + std::to_string(board.width()) + "x"
        + std::to_string(board.height()) + "  tu "
        + std::to_string(board.time_unit());
    std::string hud_players_info = "players " + std::to_string(board.players().size())
        + "  teams " + std::to_string(board.teams().size());

    ::draw::Draw_text(hud_map_info, {10, 10}, 24, DARKGRAY);
    ::draw::Draw_text(hud_players_info, {10, 40}, 24, DARKGRAY);
    if (board.game_ended())
        ::draw::Draw_text("WINNER: " + board.winner(), {10, 70}, 24, MAROON);
    if (_scene.active_meteors() > 0)
        ::draw::Draw_text("METEOR x"
            + std::to_string(_scene.active_meteors()), {10, 130}, 24, RED);
    const int floods = count_flooded(board);
    if (floods > 0)
        ::draw::Draw_text("FLOOD tiles=" + std::to_string(floods),
            {10, 160}, 24, BLUE);
    if (board.event_armed())
        ::draw::Draw_text("PLACE " + board.armed_event()
            + " - click a tile (Esc to cancel)", {10, 190}, 24, PURPLE);
}

void Renderer::storm_overlay(const board_data::BoardData &board)
{
    if (!board.storm().has_value())
        return;
    const auto &s = *board.storm();
    const std::string line = "STORM center=(" + std::to_string(s.center_x)
        + "," + std::to_string(s.center_y) + ")  R=" + std::to_string(s.radius)
        + "  dir=" + dir_char(s.direction);

    ::draw::Draw_text(line, {10, 100}, 24, ORANGE);
}

void Renderer::drain_events(board_data::BoardData &board)
{
    using Kind = board_data::GraphicalEventKind;

    for (const auto &ev : board.take_events()) {
        if (ev.second.kind == Kind::EJECT)
            _scene.trigger_eject(ev.second.player_id);
        else if (ev.second.kind == Kind::BROADCAST)
            _chat.push(ev.second.player_id, ev.second.text);
    }
}

void Renderer::render_dist_hud()
{
    const float y = static_cast<float>(GetScreenHeight()) - 40.0f;
    const int dist = _scene.render_distance();
    const std::string label = dist == 0
        ? "render dist: off"
        : "render dist: " + std::to_string(dist);

    ::draw::Draw_text(label, {10.0f, y - 26.0f}, 20, DARKGRAY);
    if (GuiButton({10, y, 30, 30}, "-"))
        _scene.set_render_distance(dist - 1);
    if (GuiButton({45, y, 30, 30}, "+"))
        _scene.set_render_distance(dist + 1);
}

void Renderer::update_cursor()
{
    const bool look = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

    if (look && !IsCursorHidden())
        DisableCursor();
    if (!look && IsCursorHidden())
        EnableCursor();
}

void Renderer::trigger_event(board_data::BoardData &board, Vector2 mouse)
{
    const auto tile = _scene.pick_tile(mouse, _camera.cam);

    if (!tile)
        return;
    board.queue_admin_command("adm_event_trigger " + board.armed_event()
        + " " + std::to_string(tile->first) + " " + std::to_string(tile->second)
        + " " + std::to_string(board.armed_radius()) + "\n");
    board.disarm_event();
}

void Renderer::handle_pick(board_data::BoardData &board, ui::AdminPanel &admin,
    ui::PlayerPanel &player)
{
    const Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE))
        board.disarm_event();
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)
        || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        || admin.blocks_point(mouse) || player.blocks_point(mouse))
        return;
    if (board.event_armed())
        return trigger_event(board, mouse);
    const auto id = _scene.pick_player(mouse, _camera.cam);
    if (!id)
        return;
    if (board.is_admin())
        board.set_selected_player(*id);
    else
        board.select_player_local(*id);
}

void Renderer::update_skybox()
{
    SkyboxUpdate(&_skybox);
}

void Renderer::draw_skybox()
{
    DrawSkyboxModel(&_skybox);
}

void Renderer::draw_crosshair()
{
    const int cx = GetScreenWidth() / 2;
    const int cy = GetScreenHeight() / 2;

    DrawLine(cx - 8, cy, cx + 8, cy, DARKGRAY);
    DrawLine(cx, cy - 8, cx, cy + 8, DARKGRAY);
}

void Renderer::draw(board_data::BoardData &board, ui::AdminPanel &admin,
    ui::PlayerPanel &player)
{
    const bool typing = admin.capturing_keyboard()
        || player.capturing_keyboard();
    _scene.sync(board);
    drain_events(board);
    update_cursor();
    update_skybox();
    _camera.set_map_size(board.width(), board.height());
    _camera.update_free_rmb(!typing);
    handle_pick(board, admin, player);
    if (IsKeyPressed(KEY_B) && !typing)
        _scene.set_show_biomes(!_scene.show_biomes());
    if (IsKeyPressed(KEY_T) && !typing)
        _scene.set_show_storm(!_scene.show_storm());
    if (IsKeyPressed(KEY_F) && !typing)
        _scene.set_show_flood(!_scene.show_flood());
    update_light();
    ::draw::Start_Draw();
    ClearBackground(RAYWHITE);
    _camera.Begin_mode_3d();
    draw_skybox();
    trantor::_health_bar_batch->draw_health_bars(_camera.cam);
    _scene.draw(board, _camera.cam);
    _camera.End_mode_3d();
    overlay(board);
    render_dist_hud();
    storm_overlay(board);
    _chat.draw();
    if (IsCursorHidden())
        draw_crosshair();
    admin.draw();
    player.draw();
    ::draw::End_Draw();
}

} // namespace zappy::gui::render
