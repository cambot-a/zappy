/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** world
*/

#include "gui/world.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

constexpr float TILE_UNIT = 6.0f;

constexpr std::array<const char *, 6> PALETTE_PATHS = {
    "src/gui/assets/farm_grass-tile.glb",
    "src/gui/assets/autumn_tile.glb",
    "src/gui/assets/farm_desert-tile.glb",
    "src/gui/assets/farm_tundra-tile.glb",
    "src/gui/assets/farm_snow-tile.glb",
    "src/gui/assets/ocean_tile.glb"
};

float wave_at(int x, int y, double time, float scale)
{
    const float phase = static_cast<float>(x + y) * 0.4f;

    return sinf(static_cast<float>(time * 3.0) + phase) * scale;
}

float wave_scale(double time)
{
    return (2.0f + static_cast<float>(sin(time))) * 0.2f;
}

} // namespace

world::world(int x, int y)
    : _width(x), _height(y)
{
    load_palette();
    _index.assign(y, std::vector<int>(x, 0));
}

world::~world()
{
}

void world::load_palette()
{
    for (const char *path : PALETTE_PATHS)
        this->_palette.push_back(std::make_unique<model>(path));
}

void world::set_tile_model(int x, int y, int model_index)
{
    if (y < 0 || y >= this->_height || x < 0 || x >= this->_width)
        return;
    if (model_index < 0 || model_index >= static_cast<int>(_palette.size()))
        return;
    this->_index[y][x] = model_index;
}

void world::set_shader(Shader shader)
{
    for (auto &tile : this->_palette)
        tile->set_shader(shader);
}

float world::get_wave_pos(int x, int y)
{
    const double time = GetTime();

    return wave_at(x, y, time, wave_scale(time));
}

void world::draw_tile(int x, int y, double time, float scale)
{
    const float wave = wave_at(x, y, time, scale);
    model *tile = this->_palette[this->_index[y][x]].get();

    tile->set_pos({static_cast<float>(x) * TILE_UNIT, wave,
        static_cast<float>(y) * TILE_UNIT});
    tile->Draw_model();
}

void world::draw_world(int cx, int cy, int radius)
{
    const double time = GetTime();
    const float scale = wave_scale(time);
    int x0 = 0;
    int y0 = 0;
    int x1 = this->_width;
    int y1 = this->_height;

    if (radius > 0) {
        x0 = std::max(0, cx - radius);
        y0 = std::max(0, cy - radius);
        x1 = std::min(this->_width, cx + radius + 1);
        y1 = std::min(this->_height, cy + radius + 1);
    }
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
            draw_tile(x, y, time, scale);
}
