/*
** EPITECH PROJECT, 2026
** zappy_raylib
** File description:
** camera
*/

#include "gui/camera.hpp"
#include "gui/utils.hpp"
#include <iostream>
#include <map>


camera::camera(Vector3 pos)
    :  cameraMode(CAMERA_FREE)
{
    this->cam.position = pos;
    this->cam.fovy = 45.0f;
    this->cam.target = NULL_VECTOR;
    this->cam.up = UP_DIRECTION;
    this->cam.projection = CAMERA_PERSPECTIVE;
}

void camera::set_mode(CameraMode mode)
{
    cameraMode = mode;
    this->cam.up = UP_DIRECTION;
}

void camera::update_camera()
{
    UpdateCamera(&this->cam, cameraMode);
}

static constexpr float BASE_SPEED = 0.3f;
static constexpr float BASE_ZOOM = 2.0f;
static constexpr float REFERENCE_MAP = 10.0f;

static Vector3 free_movement(float speed)
{
    Vector3 mov = {0.0f, 0.0f, 0.0f};

    mov.x = static_cast<float>((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        - (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))) * speed;
    mov.y = static_cast<float>((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))) * speed;
    mov.z = static_cast<float>(IsKeyDown(KEY_SPACE)
        - IsKeyDown(KEY_LEFT_CONTROL)) * speed;
    return mov;
}

void camera::set_map_size(int width, int height)
{
    const float biggest = static_cast<float>(width > height ? width : height);

    _move_scale = biggest > 0.0f ? biggest / REFERENCE_MAP : 1.0f;
}

void camera::update_free_rmb(bool allow_move)
{
    Vector3 rot = {0.0f, 0.0f, 0.0f};
    Vector3 mov = {0.0f, 0.0f, 0.0f};
    const float zoom = -GetMouseWheelMove() * BASE_ZOOM * _move_scale;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        rot.x = GetMouseDelta().x * 0.15f;
        rot.y = GetMouseDelta().y * 0.15f;
    }
    if (allow_move)
        mov = free_movement(BASE_SPEED * _move_scale);
    UpdateCameraPro(&this->cam, mov, rot, zoom);
}

void camera::set_pos(Vector3 pos)
{
    this->cam.position = pos;
}

void camera::set_target(Vector3 pos)
{
    this->cam.target = pos;
}

void camera::set_up(Vector3 pos)
{
    this->cam.up = pos;
}

void camera::set_fovy(float pos)
{
    this->cam.fovy = pos;
}

void camera::Begin_mode_3d()
{
    BeginMode3D(this->cam);
}

void camera::End_mode_3d()
{
    EndMode3D();
}
