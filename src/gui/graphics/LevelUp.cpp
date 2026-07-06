/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** LevelUp
*/

#include "gui/LevelUp.hpp"
#include "gui/utils.hpp"

LevelUp::LevelUp()
    : _pos({0, 0, 0})
{
    if (!_mod->isloaded()) {
        _mod->load();
        this->_mod->load_anim();
    }
}

void LevelUp::set_pos(Vector3 pos)
{
    this->_pos = pos;
}

void LevelUp::set_shader(Shader shader)
{
    this->_mod->set_shader(shader);
}

void LevelUp::draw()
{
    this->_angle += LEVEL_UP_SPIN;
    this->_mod->set_pos(this->_pos);
    this->_mod->set_angle(this->_angle);
    this->_mod->Draw_model();
}
