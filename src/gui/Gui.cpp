/*
** EPITECH PROJECT, 2026
** Gui.cpp
** File description:
** Top-level gui orchestrator implementation
*/

#include "gui/Gui.hpp"
#include "gui/assets.hpp"

#include <csignal>
#include <iostream>

namespace zappy::gui {

namespace {

volatile std::sig_atomic_t g_stop = 0;

void on_sigint(int)
{
    g_stop = 1;
}

} // namespace

Gui::Gui(const ipc::ServerInfo &server_info)
    : _server_info(server_info), _board(server_info)
{
}

Gui::~Gui() = default;

void Gui::connectServer()
{
    _link.emplace(_server_info.server_hostname(), _server_info.port());
    std::cerr << "[gui] connected to " << _server_info.server_hostname()
              << ":" << _server_info.port() << "\n";
}

void Gui::handleServerEvents()
{
    while (auto ev = _link->next_message())
        _board.apply(*ev);
    for (auto &cmd : _board.take_outbox())
        _link->send(std::move(cmd));
}

void Gui::run()
{
    connectServer();
    _renderer.emplace();
    _admin.emplace(*_link, _board);
    _player.emplace(_board);
    std::signal(SIGINT, on_sigint);
    while (g_stop == 0 && !_renderer->should_close()) {
        try {
            _link->poll_once(POLL_TIMEOUT_MS);
        } catch (const net::ServerLinkError &error) {
            std::cerr << "[gui] " << error.what() << "\n";
            break;
        }
        handleServerEvents();
        _renderer->draw(_board, *_admin, *_player);
    }
    release_shared_models();
}

} // namespace zappy::gui
