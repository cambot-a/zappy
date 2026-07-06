/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GuiNotifier implementation
*/

#include <utility>
#include "server/gui/GuiNotifier.hpp"

/**
 * @brief Construct a new Gui Notifier object.
 *
 * @param clients reference to the active client registry
 * @param world reference to the game world
 * @param retunePollMask callback to request adjustment of poll loop mask
 */
zappy::server::gui::GuiNotifier::GuiNotifier(
    client::ClientRegistry &clients,
    const game::World &world,
    std::function<void(int fd)> retunePollMask) noexcept
    : _clients(clients),
      _world(world),
      _lineBuilder(world),
      _retunePollMask(std::move(retunePollMask)),
      _victoryNotified(false)
{
}

/**
 * @brief Queue a message line to all GUI and GUI_ADMIN clients.
 *
 * @param line message text (without trailing newline)
 */
void zappy::server::gui::GuiNotifier::broadcast(const std::string &line)
{
    auto callback = [&line, this](client::Client &client) {
        client.buffer().queue_message(line);
        _retunePollMask(client.fd());
    };

    _clients.forEachInState(client::ClientState::GUI, callback);
    _clients.forEachInState(client::ClientState::GUI_ADMIN, callback);
}

/**
 * @brief Broadcast a server message to all GUI clients.
 *
 * @param message the message string
 */
void zappy::server::gui::GuiNotifier::broadcastMessage(
    const std::string &message)
{
    broadcast(_lineBuilder.smg(message));
}

/**
 * @brief Translate onTileChanged world events into GUI bct updates.
 *
 * @param pos coordinate of the affected tile
 */
void zappy::server::gui::GuiNotifier::onTileChanged(game::Position pos)
{
    broadcast(_lineBuilder.bct(pos.x(), pos.y()));
}

void zappy::server::gui::GuiNotifier::onPlayerAdded(int id)
{
    broadcast(_lineBuilder.pnw(id));
}

void zappy::server::gui::GuiNotifier::onPlayerMoved(int id,
    game::Position, game::Position)
{
    broadcast(_lineBuilder.ppo(id));
}

void zappy::server::gui::GuiNotifier::onPlayerRotated(int id)
{
    broadcast(_lineBuilder.ppo(id));
}

void zappy::server::gui::GuiNotifier::onPlayerLevelChanged(int id)
{
    broadcast(_lineBuilder.plv(id));
    if (_victoryNotified || _world.player(id).level() != 8)
        return;
    const std::string &teamName = _world.player(id).team();
    int count = 0;
    _world.forEachPlayer([&teamName, &count](int, const game::Player &p) {
        if (p.team() == teamName && p.level() == 8)
            count++;
    });
    if (count >= 6) {
        _victoryNotified = true;
        broadcast(_lineBuilder.seg(teamName));
    }
}

void zappy::server::gui::GuiNotifier::onPlayerInventoryChanged(int id)
{
    broadcast(_lineBuilder.pin(id));
}

void zappy::server::gui::GuiNotifier::onPlayerRemoved(int id)
{
    if (_world.hasPlayer(id) &&
        _world.player(id).state() == game::PlayerState::DEAD) {
        broadcast(_lineBuilder.pdi(id));
    }
}

void zappy::server::gui::GuiNotifier::onPlayerBroadcast(int id,
    const std::string &text)
{
    broadcast(_lineBuilder.pbc(id, text));
}

void zappy::server::gui::GuiNotifier::onPlayerForkStarted(int playerId)
{
    broadcast(_lineBuilder.pfk(playerId));
}

void zappy::server::gui::GuiNotifier::onIncantationStarted(int initiatorId,
    int level, const std::vector<int> &participants)
{
    broadcast(_lineBuilder.pic(initiatorId, level, participants));
}

void zappy::server::gui::GuiNotifier::onIncantationEnded(int initiatorId,
    bool success, int)
{
    if (_world.hasPlayer(initiatorId)) {
        broadcast(_lineBuilder.pie(
            _world.player(initiatorId).position(), success));
    }
}

void zappy::server::gui::GuiNotifier::onPlayerEjected(int id)
{
    broadcast(_lineBuilder.pex(id));
}

void zappy::server::gui::GuiNotifier::onPlayerDroppedResource(int id,
    game::ResourceType type)
{
    broadcast(_lineBuilder.pdr(id, type));
}

void zappy::server::gui::GuiNotifier::onPlayerPickedUpResource(int id,
    game::ResourceType type)
{
    broadcast(_lineBuilder.pgt(id, type));
}

/**
 * @brief Triggered when a new egg is added to the world.
 *
 * @param id egg ID
 */
void zappy::server::gui::GuiNotifier::onEggAdded(int id)
{
    broadcast(_lineBuilder.enw(id));
}

/**
 * @brief Triggered when an egg is removed from the world.
 *
 * @param id egg ID
 */
void zappy::server::gui::GuiNotifier::onEggRemoved(int id)
{
    if (_world.hasEgg(id)) {
        if (_world.egg(id).state() == game::EggState::HATCHED) {
            broadcast(_lineBuilder.ebo(id));
        } else {
            broadcast(_lineBuilder.edi(id));
        }
    }
}

void zappy::server::gui::GuiNotifier::onTileFloodChanged(
    game::Position pos, bool flooded)
{
    broadcast(_lineBuilder.evtFloodTile(pos.x(), pos.y(), flooded));
}

void zappy::server::gui::GuiNotifier::onTileBiomeChanged(
    game::Position pos, game::Biome biome)
{
    broadcast(_lineBuilder.evtBiomeSet(pos.x(), pos.y(), biome));
}
