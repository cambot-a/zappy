/*
** EPITECH PROJECT, 2026
** Item.cpp
** File description:
** Item
*/

#include "gui/Item.hpp"
#include "gui/board_data/TileData.hpp"
#include <array>
#include <cstddef>

Item::Item(const zappy::gui::board_data::Resources resources, Vector3 pos)
    : _resources(resources), _pos(pos)
{
    if (_mod_food && !_mod_food->isloaded())
        _mod_food->load();
    if (_mod_mineral && !_mod_mineral->isloaded())
        _mod_mineral->load();
}

void Item::sync(const zappy::gui::board_data::Resources resources)
{
    _resources = resources;
}

void Item::draw()
{
    if (_resources[R_FOOD] > 0 && _mod_food) {
        _mod_food->set_pos({_pos.x + FOOD_OFFSET.x,
            _pos.y + ITEM_LIFT, _pos.z + FOOD_OFFSET.z});
        _mod_food->Draw_model();
    }
    if (!_mod_mineral)
        return;
    for (std::size_t i = 0; i < MINERAL_COUNT; ++i) {
        if (_resources[R_LINEMATE + i] <= 0)
            continue;
        _mod_mineral->set_scale(MINERAL_SCALE[i]);
        _mod_mineral->change_color(MINERAL_COLOR[i]);
        _mod_mineral->set_pos({_pos.x + CRYSTAL_OFFSET[i].x,
            _pos.y + ITEM_LIFT, _pos.z + CRYSTAL_OFFSET[i].z});
        _mod_mineral->Draw_model();
    }
}

void Item::set_pos(Vector3 pos)
{
    this->_pos = pos;
}
