/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** LevelUp visual effect played on a player when its level increases
*/

#ifndef LEVELUP_HPP_
    #define LEVELUP_HPP_
    #include "model.hpp"
    #define LEVEL_UP_PATH "src/gui/assets/level_up_effect.glb"
    #define LEVEL_UP_SCALE 1.5f
    #define LEVEL_UP_SPIN 2.0f

class LevelUp {
    public:
        LevelUp();
        ~LevelUp() = default;
        void set_pos(Vector3 pos);
        void set_shader(Shader shader);
        void draw();
        static std::unique_ptr<model> _mod;
    private:
        Vector3 _pos;
        float _angle = 0.0f;
};

#endif /* !LEVELUP_HPP_ */
