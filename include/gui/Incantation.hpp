/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** Incataion
*/

#ifndef INCATAION_HPP_
    #define INCATAION_HPP_
    #define CIRCLE_PATH  "src/gui/assets/magic_circle.png"
    #include "draw.hpp"

class Incantation {
    public:
        Incantation();
        ~Incantation();
        void draw();
        void set_pos(Vector3 pos);

    protected:
    private:
        Texture2D _circle;
        Rectangle _src;
        Vector3 _pos;
        float _yaw   = 0.0f;   // rotate around Y
        float _pitch = -90.0f;   // tilt up/down
        float _roll  = 0.0f;
};

#endif /* !INCATAION_HPP_ */
