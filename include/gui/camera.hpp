/*
** EPITECH PROJECT, 2026
** zappy_raylib
** File description:
** camera
*/

#ifndef CAMERA_HPP_
    #define CAMERA_HPP_
    #include <raylib.h>
    #include <raymath.h>

class camera {
    public:
        Camera cam;
        camera(Vector3 pos);
        ~camera() = default;
        void set_mode(CameraMode mode);
        void update_camera();
        void update_free_rmb(bool allow_move);
        void set_map_size(int width, int height);
        void set_pos(Vector3 pos);
        void set_target(Vector3 pos);
        void set_up(Vector3 pos);
        void set_fovy(float pos);
        void Begin_mode_3d();
        void End_mode_3d();
        void update_field();
    protected:
    private:
        int cameraMode;
        float _move_scale = 1.0f;
};

#endif /* !CAMERA_HPP_ */
