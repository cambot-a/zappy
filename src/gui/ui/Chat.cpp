/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Fading broadcast chat overlay implementation
*/

#include "gui/ui/Chat.hpp"

#include <cstddef>
#include <string>

#include <raylib.h>

#include "gui/draw.hpp"

namespace zappy::gui::ui {

namespace {

constexpr std::size_t MAX_LINES = 8;
constexpr double FADE_START = 6.0;   // seconds before a line starts fading
constexpr double LIFETIME = 9.0;     // seconds before a line disappears
constexpr float LINE_HEIGHT = 24.0f;
constexpr int FONT_SIZE = 20;
constexpr float MARGIN_X = 12.0f;
constexpr float BOTTOM_OFFSET = 85.0f; // leave room for the render-dist HUD

unsigned char line_alpha(double age)
{
    if (age < FADE_START)
        return 255;
    const double k = 1.0 - (age - FADE_START) / (LIFETIME - FADE_START);
    return static_cast<unsigned char>(255.0 * (k < 0.0 ? 0.0 : k));
}

} // namespace

void Chat::push(int player_id, const std::string &text)
{
    _lines.push_back({"#" + std::to_string(player_id) + ": " + text,
        GetTime()});
    if (_lines.size() > MAX_LINES)
        _lines.pop_front();
}

void Chat::draw()
{
    const double now = GetTime();

    while (!_lines.empty() && now - _lines.front().time >= LIFETIME)
        _lines.pop_front();

    const float bottom = static_cast<float>(GetScreenHeight()) - BOTTOM_OFFSET;
    const int n = static_cast<int>(_lines.size());
    for (int i = 0; i < n; ++i) {
        const Line &l = _lines[static_cast<std::size_t>(i)];
        const float y = bottom - static_cast<float>(n - 1 - i) * LINE_HEIGHT;
        const Color c = {235, 235, 240, line_alpha(now - l.time)};
        ::draw::Draw_text(l.text, {MARGIN_X, y}, FONT_SIZE, c);
    }
}

} // namespace zappy::gui::ui
