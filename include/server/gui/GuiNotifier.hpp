/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GuiNotifier class translating world events into GUI broadcasts
*/

#ifndef SERVER_GUI_GUINOTIFIER_HPP_
    #define SERVER_GUI_GUINOTIFIER_HPP_

    #include <functional>
    #include <string>
    #include "server/game/IWorldObserver.hpp"
    #include "server/client/ClientRegistry.hpp"
    #include "server/gui/GuiLineBuilder.hpp"

namespace zappy::server::gui {

/**
 * @brief Listens to World mutations as an IWorldObserver and broadcasts
 *        equivalent protocol notifications to all connected GUI clients.
 */
class GuiNotifier : public game::WorldObserverAdapter {
public:
    /**
     * @brief Construct a new Gui Notifier.
     *
     * @param clients reference to the active client registry
     * @param world reference to the game world
     * @param retunePollMask callback to request adjustment of poll loop mask
     */
    GuiNotifier(client::ClientRegistry &clients,
                const game::World &world,
                std::function<void(int fd)> retunePollMask) noexcept;

    GuiNotifier(const GuiNotifier &) = delete;
    GuiNotifier &operator=(const GuiNotifier &) = delete;
    GuiNotifier(GuiNotifier &&) noexcept = default;
    GuiNotifier &operator=(GuiNotifier &&) noexcept = default;

    ~GuiNotifier() override = default;

    /**
     * @brief Queue a message line to all GUI and GUI_ADMIN clients.
     *
     * @param line message text (without trailing newline)
     */
    void broadcast(const std::string &line);

    /**
     * @brief Broadcast a server message to all GUI clients.
     *
     * @param message the message string
     */
    void broadcastMessage(const std::string &message);

    /**
     * @brief Triggered when a tile's resource layout is modified.
     *
     * @param pos coordinate of the affected tile
     */
    void onTileChanged(game::Position pos) override;

    void onPlayerAdded(int id) override;
    void onPlayerMoved(int id, game::Position oldPos,
        game::Position newPos) override;
    void onPlayerRotated(int id) override;
    void onPlayerLevelChanged(int id) override;
    void onPlayerInventoryChanged(int id) override;
    void onPlayerRemoved(int id) override;
    void onPlayerBroadcast(int id, const std::string &text) override;
    void onPlayerForkStarted(int playerId) override;
    void onIncantationStarted(int initiatorId, int level,
        const std::vector<int> &participants) override;
    void onIncantationEnded(int initiatorId, bool success,
        int newLevel) override;
    void onPlayerEjected(int id) override;
    void onPlayerDroppedResource(int id, game::ResourceType type) override;
    void onPlayerPickedUpResource(int id, game::ResourceType type) override;
    void onEggAdded(int id) override;
    void onEggRemoved(int id) override;
    void onTileFloodChanged(game::Position pos, bool flooded) override;
    void onTileBiomeChanged(game::Position pos, game::Biome biome) override;

private:
    client::ClientRegistry &_clients;
    const game::World &_world;
    GuiLineBuilder _lineBuilder;
    std::function<void(int fd)> _retunePollMask;
    bool _victoryNotified;
};

} // namespace zappy::server::gui

#endif /* !SERVER_GUI_GUINOTIFIER_HPP_ */
