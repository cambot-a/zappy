/*
** EPITECH PROJECT, 2026
** zappy_raylib
** File description:
** window
*/

#include "gui/window.hpp"

window::window(int widght, int height, std::string name)
{
    InitWindow(widght, height, name.c_str());
}

window::~window()
{
        CloseWindow();
}

bool window::WindowIsCLose()
{
    return WindowShouldClose();
}

void window::Set_window_state(ConfigFlags flags)
{
    if (IsWindowState(flags))
        ClearWindowState(flags);
    else
        SetWindowState(flags);
}
