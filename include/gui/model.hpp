/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** model
*/

#ifndef MODEL_HPP_
    #define MODEL_HPP_
    #include <iostream>
    #include <memory>
    #include <raylib.h>
    #include <raymath.h>

class model{
    public:
        model(std::string name, bool load, Vector3 scale, Vector3 pos = {0.0f, 0.0f, 0.0f});
        model(std::string name = "", bool load = true, double scale = 1.0f, Vector3 pos = {0.0f, 0.0f, 0.0f})
            : model(name, load, Vector3Scale({ 1.0f, 1.0f, 1.0f }, scale), pos) {};
        ~model();
        model(const model &) = delete;
        model &operator=(const model &) = delete;
        bool isloaded();
        void Update_anim();
        void load_anim();
        void load_texture();
        void load_texture(std::string name);
        void switch_index_anime();
        void Draw_model(int current_frame = -1);
        void set_angle(int angle) { _rotationAngle = angle; }
        void set_scale(double scale) { _scale = Vector3Scale({ 1.0f, 1.0f, 1.0f }, scale); }
        void set_scale(Vector3 scale) { _scale = scale; }
        void change_color(Color cl);
        void set_shader(Shader shader);
        Color get_color();
        void move_model(Vector3 rotationAxis, float rotationAngle, Vector3 scale);
        void set_pos(Vector3 pos);
        [[nodiscard]] BoundingBox get_bbox();
        void set_base_pos(Vector3 pos);
        Vector3 get_pos();
        Vector3 get_base_pos();
        void set_anim_index(int anime_index);
        void load();
        int get_frame_count();
        int get_current_frame();
        //void Load_model(std::string name);
        //void update_model_anim();
    protected:
    private:
        Model _model{};
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
        Vector3 _rotationAxis;
        float _rotationAngle;
        Vector3 _scale;

        void check_frame();
};

#endif /* !MODEL_HPP_ */
