/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** Egg
*/

#ifndef EGG_HPP_
    #define EGG_HPP_
    #include "model.hpp"
    #define EGG_PATH "src/gui/assets/egg.glb"

class Egg {
    public:
        Egg();
        ~Egg() = default;
        void draw();
        void set_pos(Vector3);
        static std::unique_ptr<model> _mod;
    private:
        Vector3 _pos;
};
#define EGG_SCALE 0.5

#endif /* !EGG_HPP_ */
