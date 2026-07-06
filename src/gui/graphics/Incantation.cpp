/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** Incantation
*/

#include "gui/Incantation.hpp"
#include "gui/utils.hpp"

Incantation::Incantation()
    : _circle(LoadTexture(CIRCLE_PATH)),
    _src({ 0, 0, static_cast<float>(_circle.width),
        static_cast<float>(_circle.height)}),
         _pos({0, 0, 0}),
    _yaw(0.0f), _pitch(-90.0f), _roll(0.0f)
{
}

Incantation::~Incantation()
{
    UnloadTexture(this->_circle);
}

void Incantation::draw()
{
    this->_yaw++;
    float y = DEG2RAD * _yaw;
    float p = DEG2RAD * _pitch;
    float r = DEG2RAD * _roll;
    Vector3 normal = {
        sinf(y) * cosf(p),
        sinf(p),
        cosf(y) * cosf(p)
    };
    Vector3 worldUp = { sinf(r), cosf(r), 0.0f };

    draw::DrawTextureIn3D(this->_circle,
                        UP_DIRECTION,
                        normal, worldUp,
                        8.0f, 8.0f, _src, WHITE);
}

void Incantation::set_pos(Vector3 pos)
{
    pos.y = 1;
    this->_pos = pos;
}
