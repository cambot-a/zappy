/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** trantor
*/

#ifndef TRANTOR_HPP_
    #define TRANTOR_HPP_
    #include "model.hpp"
    #include "gui/HealthBarBatch.hpp"
    #define TRANTOR_PATH "src/gui/assets/robot.glb"
    #include "gui/board_data/BoardData.hpp"

using TrantorState = enum TRANTOR_STATE {
    DANCE,
    DIE,
    LOOK_ARROUND,
    JUMP,
    HEAD_NO,
    PUNCH,
    RUN,
    SIT,
    STAND_UP,
    HAND_SHAKE,
    WALK,
    WALK_JUMP,
    WAVE,
    YES,
};

#define DEFAULT_TRANTORIAN_SCALE CLITERAL(Vector3){0.5, 0.5, 0.5}
class trantor {
    public:
        trantor(const zappy::gui::board_data::PlayerData& player_data, Vector3 player_pos);
        ~trantor() = default;
        void Turn_left();
        void Turn_right();
        void look_around();
        void eject_player();
        void walk();
        void death();
        void sync( const zappy::gui::board_data::PlayerData& player_data, Vector3 player_pos);
        void set_pos(Vector3 pos);
        void set_shader(Shader shader);
        void Update_anim();
        void draw();
        static std::unique_ptr<model> _mod;
        static std::unique_ptr<HealthBarBatch> _health_bar_batch;
    protected:
    private:
        int _id;
        Vector3 _pos;
        TrantorState _state;
        int _angle;
        double _health;
        bool _is_alive;
        int _death_frame;
};

#endif /* !TRANTOR_HPP_ */
