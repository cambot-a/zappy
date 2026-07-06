/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** assets
*/

#ifndef ASSETS_HPP_
    #define ASSETS_HPP_

// Release the shared static models (trantor/Item/Egg) while the GL context
// is still alive. Must be called before the window is closed, otherwise their
// static-duration destructors run UnloadModel after CloseWindow -> segfault.
void release_shared_models();

#endif /* !ASSETS_HPP_ */
