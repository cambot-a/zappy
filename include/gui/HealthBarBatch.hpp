/*
** EPITECH PROJECT, 2026
** HealthBarBatch.hpp
** File description:
** HealthBarBatch
*/

#ifndef HEALTHBARBATCH_HPP_
    #define HEALTHBARBATCH_HPP_

#include <iostream>
    #include <memory>
    #include <raylib.h>
    #include <raymath.h>
    #include "gui/utils.hpp"
    #include <map>
    #include <vector>
    #define TMP_HEALTH_BAR_TEXTURE "src/gui/assets/red.png"
    #define HEALTHBAR_WIDTH 0.028f
    #define HEALTHBAR_THICKNESS 3.25f
    #define HEALTHBAR_HEIGHT (HEALTHBAR_WIDTH * HEALTHBAR_THICKNESS)
    #define HEALTHBAR_HEIGHT_MODIFIER 3.0f
    #define UP_V2 {0, 1}

class HealthBarBatch {
    public:
        HealthBarBatch(bool load = true, float scale = 1.0f);
        ~HealthBarBatch();
        bool isloaded();
        void draw_health_bars(Camera camera);
        void scale_health_bars(double _scale);
        void sync_health_bar(int id, Vector3 pos, double percentage);
        void kill(int id);
        void load();
        HealthBarBatch(const HealthBarBatch &) = delete;
        HealthBarBatch &operator=(const HealthBarBatch &) = delete;
        class HealthBarInfo
        {
            public:
            HealthBarInfo(Vector3 _position, double _percentage)
                : position(_position), percentage(_percentage) {}
            void set(double _percentage) { percentage = _percentage; }
            void set(Vector3 _position, double _percentage = INFINITY)
            {
                position = _position;
                if (percentage != INFINITY)
                    percentage = _percentage;
            }
            Vector3 position;
            double percentage;
        };
    private:
        void draw_health_bar_unit(HealthBarInfo healthbar, Camera camera);
        std::vector<HealthBarInfo> get_order(Vector3 camera_position);
        float _scale;
        Texture2D _texture;
        std::map<int, HealthBarInfo> healthbars_info;
        bool _is_loaded;
    /*
        ModelAnimation *_anim;
        int _animeCount;
        int _current_frame;
        int _anim_index;
        Color _color;
        std::string _name;
        Vector3 _pos;
        Vector3 _base_pos;
        Texture2D _texture;
        bool loaded;
*/
};

#endif /* HEALTHBARBATCH_HPP_ */
