/*
** EPITECH PROJECT, 2026
** zappy_raylib
** File description:
** window
*/

#ifndef WINDOW_HPP_
    #define WINDOW_HPP_
    #include <raylib.h>
    #include <iostream>
    #include <raymath.h>

class window {
    public:
        window(int widght, int height, std::string name);
        ~window();
        void Set_window_state(ConfigFlags flags);
        bool WindowIsCLose();
    protected:
    private:

};

#endif /* !WINDOW_HPP_ */
