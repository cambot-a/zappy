/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** cube
*/

#ifndef CUBE_HPP_
    #define CUBE_HPP_
    #include <raylib.h>
    #include <raymath.h>

class cube {
    public:
        cube();
        ~cube() = default;
        void set_pos(Vector3 pos);
        void set_color(Color color);
        void Draw_cube();
        void Draw_cube_wire();
    protected:
    private:
        Vector3 _pos;
        Color _color;
};

#endif /* !CUBE_HPP_ */
