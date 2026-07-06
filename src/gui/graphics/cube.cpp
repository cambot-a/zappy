/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** cube
*/

#include "gui/cube.hpp"

cube::cube()
    : _pos({0, 0, 0}), _color(RED)
{

}

void cube::Draw_cube()
{
    DrawCube(this->_pos, 1, 1, 1, this->_color);
}

void cube::Draw_cube_wire()
{
    DrawCubeWires(this->_pos, 1, 1, 1, BLACK);
}

void cube::set_color(Color color)
{
    this->_color = color;
}

void cube::set_pos(Vector3 pos)
{
    this->_pos = pos;
}
