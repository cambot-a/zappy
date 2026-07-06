/*
** EPITECH PROJECT, 2026
** Gui.hpp
** File description:
** Top-level gui orchestrator
*/

#ifndef GUI_HPP_
    #define GUI_HPP_
    #include <optional>

    #include "gui/board_data/BoardData.hpp"
    #include "gui/ipc/ServerInfo.hpp"
    #include "gui/net/ServerLink.hpp"
    #include "gui/render/Renderer.hpp"
    #include "gui/ui/AdminPanel.hpp"
    #include "gui/ui/PlayerPanel.hpp"

namespace zappy::gui {

class Gui {
public:
    explicit Gui(const ipc::ServerInfo &server_info);
    ~Gui();

    Gui(const Gui &) = delete;
    Gui &operator=(const Gui &) = delete;

    void run();

    [[nodiscard]] const board_data::BoardData &board() const noexcept
        { return _board; }

private:
    void connectServer();
    void handleServerEvents();

    static constexpr int POLL_TIMEOUT_MS = 0;

    ipc::ServerInfo _server_info;
    board_data::BoardData _board;
    std::optional<net::ServerLink> _link;
    std::optional<render::Renderer> _renderer;
    std::optional<ui::AdminPanel> _admin;
    std::optional<ui::PlayerPanel> _player;
};

} // namespace zappy::gui

#endif /* GUI_HPP_ */
