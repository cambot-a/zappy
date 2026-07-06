/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** Egg
*/

#include "gui/Egg.hpp"

Egg::Egg() : _pos( {0, 0, 0} )
{
    if (!_mod->isloaded())
        _mod->load();
}

void Egg::draw()
{
    this->_mod->set_pos(_pos);
    this->_mod->Draw_model();
}

void Egg::set_pos(Vector3 pos)
{
    pos.y += 0.5;
    _pos = pos;
}
