/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GuiDispatcher implementation
*/

#include <optional>
#include <sstream>
#include <utility>

#include "server/gui/GuiDispatcher.hpp"
#include "server/game/ElevationRules.hpp"
#include "server/game/ResourceNameResolver.hpp"
#include "server/event/EventBus.hpp"
#include "server/event/Storm.hpp"
#include "server/event/Flood.hpp"
#include "server/event/Meteor.hpp"

/**
 * @brief Construct a new Gui Dispatcher object.
 *
 * @param world the game world reference
 * @param adminDeps client registry, feature flags and admin password
 * @param onResponse the callback to dispatch responses
 * @param getFrequency callback to query the current frequency
 * @param setFrequency callback to update the time frequency
 */
zappy::server::gui::GuiDispatcher::GuiDispatcher(
    game::World &world, GuiAdminDeps adminDeps,
    std::function<void(int fd, std::string)> onResponse,
    std::function<int()> getFrequency,
    std::function<void(int)> setFrequency)
    : _world(world),
      _onResponse(std::move(onResponse)),
      _lineBuilder(world),
      _getFrequency(std::move(getFrequency)),
      _setFrequency(std::move(setFrequency)),
      _clients(adminDeps.clients),
      _featureFlags(adminDeps.featureFlags),
      _adminPassword(std::move(adminDeps.adminPassword)),
      _onPlayerKilledByAdmin(std::move(adminDeps.onPlayerKilledByAdmin)),
      _eventBus(adminDeps.eventBus),
      _onStartIncantation(std::move(adminDeps.onStartIncantation)),
      _onStopIncantation(std::move(adminDeps.onStopIncantation))
{
}

/**
 * @brief Run a standard integer-argument command through its handler.
 *
 * @param cmd the parsed command (never ADMIN)
 * @return std::vector<std::string> the response lines
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::dispatchStandard(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    static constexpr std::array<Handler, static_cast<std::size_t>(
        protocol::gui::GuiCommandKind::ADMIN)> handlers = {
            &GuiDispatcher::handleMsz, &GuiDispatcher::handleBct,
            &GuiDispatcher::handleMct, &GuiDispatcher::handleTna,
            &GuiDispatcher::handlePpo, &GuiDispatcher::handlePlv,
            &GuiDispatcher::handlePin, &GuiDispatcher::handleSgt,
            &GuiDispatcher::handleSst, &GuiDispatcher::handlePpf};
    const std::size_t idx = static_cast<std::size_t>(cmd.kind);

    return (this->*handlers[idx])(cmd.arguments);
}

/**
 * @brief Dispatch a raw command line from a specific client.
 *
 * @param fd client file descriptor
 * @param line raw command line
 */
void zappy::server::gui::GuiDispatcher::dispatch(
    int fd, std::string_view line)
{
    const auto res = protocol::gui::GuiCommandParser::parse(line);

    if (std::holds_alternative<protocol::gui::GuiParseError>(res)) {
        sendError(fd, std::get<protocol::gui::GuiParseError>(res));
    } else {
        const auto responses = routeCommand(
            fd, std::get<protocol::gui::ParsedGuiCommand>(res));
        for (const auto &response : responses)
            _onResponse(fd, response);
    }
}

/**
 * @brief Route a parsed command to admin, admin-scoped or standard handling.
 *
 * @param fd client file descriptor
 * @param cmd the parsed command
 * @return std::vector<std::string> the response lines
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::routeCommand(
    int fd, const protocol::gui::ParsedGuiCommand &cmd)
{
    std::vector<std::string> responses;

    if (cmd.kind == protocol::gui::GuiCommandKind::ADMIN)
        responses = handleAdmin(fd, cmd);
    else if (isAdminCommand(cmd.kind))
        responses = handleAdminScopedCommand(fd, cmd);
    else
        responses = dispatchStandard(cmd);
    return responses;
}

/**
 * @brief Execute an adm_* command, gated on the client being GUI_ADMIN.
 *
 * @param fd client file descriptor
 * @param cmd the parsed adm_* command
 * @return std::vector<std::string> the response lines
 */
std::vector<std::string>
zappy::server::gui::GuiDispatcher::handleAdminScopedCommand(
    int fd, const protocol::gui::ParsedGuiCommand &cmd)
{
    static constexpr std::array<AdminHandler, protocol::gui::
        GUI_COMMAND_KIND_COUNT - static_cast<std::size_t>(
            protocol::gui::GuiCommandKind::ADM_FLAG_ENABLE)> handlers = {
        &GuiDispatcher::handleAdmFlagEnable,
        &GuiDispatcher::handleAdmFlagDisable,
        &GuiDispatcher::handleAdmFlagList,
        &GuiDispatcher::handleAdmEventTrigger,
        &GuiDispatcher::handleAdmTileSet,
        &GuiDispatcher::handleAdmTileAdd,
        &GuiDispatcher::handleAdmPlayerKill,
        &GuiDispatcher::handleAdmPlayerTp,
        &GuiDispatcher::handleAdmPlayerLevel,
        &GuiDispatcher::handleAdmPlayerIncant,
        &GuiDispatcher::handleAdmPlayerStopIncant};
    const std::size_t idx = static_cast<std::size_t>(cmd.kind)
        - static_cast<std::size_t>(
            protocol::gui::GuiCommandKind::ADM_FLAG_ENABLE);
    std::vector<std::string> responses = {"suc"};

    if (_clients.get(fd).state() == client::ClientState::GUI_ADMIN)
        responses = (this->*handlers[idx])(cmd);
    return responses;
}

/**
 * @brief Promote the GUI client at @p fd to GUI_ADMIN if eligible.
 *
 * @param fd client file descriptor
 * @return bool true if the client was GUI/GUI_ADMIN and is now admin
 */
bool zappy::server::gui::GuiDispatcher::promoteClientAdmin(int fd)
{
    client::Client &client = _clients.get(fd);
    const bool eligible = client.state() == client::ClientState::GUI
        || client.state() == client::ClientState::GUI_ADMIN;

    if (eligible)
        client.promoteToAdmin();
    return eligible;
}

/**
 * @brief Handle the admin authentication command for @p fd.
 *
 * @param fd client file descriptor
 * @param cmd the parsed admin command (rawArgument holds the password)
 * @return std::vector<std::string> the single response line
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmin(
    int fd, const protocol::gui::ParsedGuiCommand &cmd)
{
    std::vector<std::string> response = {"ko"};

    if (!_featureFlags.isEnabled(config::FeatureFlag::ADMIN))
        response = {"suc"};
    else if (cmd.rawArgument.empty())
        response = {"sbp"};
    else if (cmd.rawArgument == _adminPassword && promoteClientAdmin(fd))
        response = {"ok"};
    return response;
}

/**
 * @brief Send appropriate error message to the client on parse error.
 *
 * @param fd client file descriptor
 * @param err type of parsing error encountered
 */
void zappy::server::gui::GuiDispatcher::sendError(
    int fd, protocol::gui::GuiParseError err)
{
    const bool isUnknown =
        (err == protocol::gui::GuiParseError::UNKNOWN_COMMAND);
    _onResponse(fd, isUnknown ? "suc" : "sbp");
}

/**
 * @brief Build the "msz <width> <height>" line for the current world.
 *
 * @return std::string the formatted map-size line
 */
std::string zappy::server::gui::GuiDispatcher::buildMszLine() const
{
    return "msz " + std::to_string(_world.width()) + " "
        + std::to_string(_world.height());
}

/**
 * @brief Handle the msz command: report the map size.
 *
 * @param args command arguments (unused)
 * @return std::vector<std::string> a single "msz <w> <h>" line
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleMsz(
    const std::vector<int> &args)
{
    (void)args;
    return {buildMszLine()};
}

/**
 * @brief Handle the bct command: report a single tile's content.
 *
 * @param args command arguments: x then y
 * @return std::vector<std::string> the bct line, or "sbp" if out of bounds
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleBct(
    const std::vector<int> &args)
{
    const int x = args[0];
    const int y = args[1];
    const bool outOfBounds = (x >= _world.width() || y >= _world.height());

    return outOfBounds ? std::vector<std::string>{"sbp"}
        : std::vector<std::string>{_lineBuilder.bct(x, y)};
}

/**
 * @brief Handle the mct command: report every tile's content.
 *
 * @param args command arguments (unused)
 * @return std::vector<std::string> one bct line per tile, Y outer, X inner
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleMct(
    const std::vector<int> &args)
{
    std::vector<std::string> lines;

    (void)args;
    for (int y = 0; y < _world.height(); ++y)
        for (int x = 0; x < _world.width(); ++x)
            lines.push_back(_lineBuilder.bct(x, y));
    return lines;
}

/**
 * @brief Handle the tna command: report every team name.
 *
 * @param args command arguments (unused)
 * @return std::vector<std::string> one "tna <name>" line per team
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleTna(
    const std::vector<int> &args)
{
    std::vector<std::string> lines;

    (void)args;
    for (const auto &team : _world.teams())
        lines.push_back("tna " + team.name());
    return lines;
}

/**
 * @brief Handle the ppo command: report a player's position and orientation.
 *
 * @param args command arguments: the player id
 * @return std::vector<std::string> the ppo line, or "sbp" if unknown
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handlePpo(
    const std::vector<int> &args)
{
    const int id = args[0];

    return _world.hasPlayer(id) ?
        std::vector<std::string>{_lineBuilder.ppo(id)}
        : std::vector<std::string>{"sbp"};
}

/**
 * @brief Handle the plv command: report a player's level.
 *
 * @param args command arguments: the player id
 * @return std::vector<std::string> the plv line, or "sbp" if unknown
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handlePlv(
    const std::vector<int> &args)
{
    const int id = args[0];

    return _world.hasPlayer(id) ?
        std::vector<std::string>{_lineBuilder.plv(id)}
        : std::vector<std::string>{"sbp"};
}

/**
 * @brief Handle the pin command: report a player's position and inventory.
 *
 * @param args command arguments: the player id
 * @return std::vector<std::string> the pin line, or "sbp" if unknown
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handlePin(
    const std::vector<int> &args)
{
    const int id = args[0];

    return _world.hasPlayer(id) ?
        std::vector<std::string>{_lineBuilder.pin(id)}
        : std::vector<std::string>{"sbp"};
}

/**
 * @brief Handle sgt command.
 *
 * @param args command arguments
 * @return std::vector<std::string> stub response
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleSgt(
    const std::vector<int> &args)
{
    (void)args;
    return {"sgt " + std::to_string(_getFrequency())};
}

/**
 * @brief Handle sst command.
 *
 * @param args command arguments
 * @return std::vector<std::string> response (empty if success, sbp on error)
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleSst(
    const std::vector<int> &args)
{
    if (args.empty() || args[0] <= 0)
        return {"sbp"};
    _setFrequency(args[0]);
    return {};
}

/**
 * @brief Handle the ppf command: aggregated player profile snapshot.
 *
 * @param args command arguments: the player id
 * @return std::vector<std::string> the ppf line, "suc" or "sbp"
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handlePpf(
    const std::vector<int> &args)
{
    std::vector<std::string> response = {"sbp"};

    if (!_featureFlags.isEnabled(config::FeatureFlag::PROFILE))
        response = {"suc"};
    else if (_world.hasPlayer(args[0]))
        response = {_lineBuilder.ppf(args[0])};
    return response;
}

/**
 * @brief Send the complete initial state sync to a promoted GUI client.
 *
 * @param fd client file descriptor
 * @param frequency current server clock frequency
 */
void zappy::server::gui::GuiDispatcher::sendInitialSync(
    int fd, int frequency)
{
    sendSyncMapSize(fd);
    sendSyncTiles(fd);
    sendSyncTeams(fd);
    sendSyncPlayers(fd);
    sendSyncEggs(fd);
    sendSyncTime(fd, frequency);
    sendSyncBiomes(fd);
}

/**
 * @brief Send the map size to the client.
 *
 * @param fd client file descriptor
 */
void zappy::server::gui::GuiDispatcher::sendSyncMapSize(int fd)
{
    _onResponse(fd, buildMszLine());
}

/**
 * @brief Send all tiles to the client.
 *
 * @param fd client file descriptor
 */
void zappy::server::gui::GuiDispatcher::sendSyncTiles(int fd)
{
    for (int y = 0; y < _world.height(); ++y) {
        for (int x = 0; x < _world.width(); ++x) {
            sendSyncTile(fd, x, y);
        }
    }
}

/**
 * @brief Send content of a single tile.
 *
 * @param fd client file descriptor
 * @param x tile horizontal coordinate
 * @param y tile vertical coordinate
 */
void zappy::server::gui::GuiDispatcher::sendSyncTile(
    int fd, int x, int y)
{
    _onResponse(fd, _lineBuilder.bct(x, y));
}

/**
 * @brief Send one evt_biome_set line per non-PLAIN tile to the client.
 *
 * @param fd client file descriptor
 */
void zappy::server::gui::GuiDispatcher::sendSyncBiomes(int fd)
{
    for (int y = 0; y < _world.height(); ++y)
        for (int x = 0; x < _world.width(); ++x)
            sendSyncBiomeTile(fd, x, y);
}

/**
 * @brief Send the biome line for a single tile, skipping PLAIN tiles.
 *
 * @param fd client file descriptor
 * @param x tile horizontal coordinate
 * @param y tile vertical coordinate
 */
void zappy::server::gui::GuiDispatcher::sendSyncBiomeTile(
    int fd, int x, int y)
{
    const game::Biome biome = _world.tileAt(game::Position(x, y)).biome();

    if (biome != game::Biome::PLAIN)
        _onResponse(fd, _lineBuilder.evtBiomeSet(x, y, biome));
}

/**
 * @brief Send all team names to the client.
 *
 * @param fd client file descriptor
 */
void zappy::server::gui::GuiDispatcher::sendSyncTeams(int fd)
{
    for (const auto &team : _world.teams()) {
        _onResponse(fd, "tna " + team.name());
    }
}

/**
 * @brief Send all player details to the client.
 *
 * @param fd client file descriptor
 */
void zappy::server::gui::GuiDispatcher::sendSyncPlayers(int fd)
{
    _world.forEachPlayer([this, fd](int id, const game::Player &player) {
        (void)id;
        if (player.state() == game::PlayerState::DEAD)
            return;
        sendSyncPlayer(fd, player);
    });
}

/**
 * @brief Send details of a single player.
 *
 * @param fd client file descriptor
 * @param player player to sync
 */
void zappy::server::gui::GuiDispatcher::sendSyncPlayer(
    int fd, const game::Player &player)
{
    const int pid = player.id();
    const int px = player.position().x();
    const int py = player.position().y();
    const int orient = static_cast<int>(player.orientation());
    const int lvl = player.level();
    const std::string pnw = "pnw " + std::to_string(pid) + " "
        + std::to_string(px) + " " + std::to_string(py) + " "
        + std::to_string(orient) + " " + std::to_string(lvl) + " "
        + player.team();
    _onResponse(fd, pnw);
    _onResponse(fd, "plv " + std::to_string(pid) + " " + std::to_string(lvl));
    sendSyncPlayerInventory(fd, player);
}

/**
 * @brief Send a player's inventory content.
 *
 * @param fd client file descriptor
 * @param player player to sync inventory for
 */
void zappy::server::gui::GuiDispatcher::sendSyncPlayerInventory(
    int fd, const game::Player &player)
{
    const int pid = player.id();
    const int px = player.position().x();
    const int py = player.position().y();
    const std::string msg = "pin " + std::to_string(pid) + " "
        + std::to_string(px) + " " + std::to_string(py) + " "
        + std::to_string(player.resource(game::ResourceType::FOOD)) + " "
        + std::to_string(player.resource(game::ResourceType::LINEMATE)) + " "
        + std::to_string(player.resource(game::ResourceType::DERAUMERE)) + " "
        + std::to_string(player.resource(game::ResourceType::SIBUR)) + " "
        + std::to_string(player.resource(game::ResourceType::MENDIANE)) + " "
        + std::to_string(player.resource(game::ResourceType::PHIRAS)) + " "
        + std::to_string(player.resource(game::ResourceType::THYSTAME));
    _onResponse(fd, msg);
}

/**
 * @brief Send all eggs to the client.
 *
 * @param fd client file descriptor
 */
void zappy::server::gui::GuiDispatcher::sendSyncEggs(int fd)
{
    _world.forEachEgg([this, fd](int id, const game::Egg &) {
        _onResponse(fd, _lineBuilder.enw(id));
    });
}

/**
 * @brief Send time frequency to the client.
 *
 * @param fd client file descriptor
 * @param frequency clock frequency of the server
 */
void zappy::server::gui::GuiDispatcher::sendSyncTime(
    int fd, int frequency)
{
    _onResponse(fd, "sgt " + std::to_string(frequency));
}

/**
 * @brief Handle adm_flag_enable: enable a feature flag by name.
 *
 * BIOMES is boot-only terrain generation: toggling it at runtime cannot
 * regenerate the map, so the request is rejected with "ko".
 *
 * @param cmd parsed command; rawArgument holds the flag name
 * @return std::vector<std::string> "ok", or "ko" if unknown or boot-only
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmFlagEnable(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const auto flag = config::FeatureFlags::fromName(cmd.rawArgument);
    std::vector<std::string> response = {"ko"};

    if (flag && *flag != config::FeatureFlag::BIOMES) {
        _featureFlags.enable(*flag);
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_flag_disable: disable a feature flag by name.
 *
 * BIOMES is boot-only terrain generation: toggling it at runtime cannot
 * undo the generated map, so the request is rejected with "ko".
 *
 * @param cmd parsed command; rawArgument holds the flag name
 * @return std::vector<std::string> "ok", or "ko" if unknown or boot-only
 */
std::vector<std::string>
zappy::server::gui::GuiDispatcher::handleAdmFlagDisable(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const auto flag = config::FeatureFlags::fromName(cmd.rawArgument);
    std::vector<std::string> response = {"ko"};

    if (flag && *flag != config::FeatureFlag::BIOMES) {
        _featureFlags.disable(*flag);
        if (*flag == config::FeatureFlag::EVENTS)
            _eventBus.cancelAll();
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_flag_list: report every flag and its on/off state.
 *
 * @param cmd parsed command (unused)
 * @return std::vector<std::string> one "adm_flag_list <name> <on|off>" line
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmFlagList(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    std::vector<std::string> lines;

    (void)cmd;
    for (const auto &[flag, on] : _featureFlags.snapshot())
        lines.push_back("adm_flag_list "
            + std::string(config::FeatureFlags::toName(flag))
            + (on ? " on" : " off"));
    return lines;
}

/**
 * @brief Build the event matching @p name, if any concrete type exists yet.
 *
 * @param name requested event name
 * @return std::unique_ptr<event::Event> the event, or nullptr if unknown
 */
std::unique_ptr<zappy::server::event::Event>
zappy::server::gui::GuiDispatcher::makeEvent(std::string_view name,
    std::optional<game::Position> center, std::optional<int> radius) const
{
    const game::Position def(_world.width() / 2, _world.height() / 2);

    if (name == "storm")
        return std::make_unique<event::Storm>(center.value_or(def),
            radius.value_or(STORM_DEFAULT_RADIUS), game::Orientation::EAST,
            STORM_DEFAULT_PUSH_INTERVAL, STORM_DEFAULT_DURATION);
    if (name == "meteor")
        return std::make_unique<event::Meteor>(center.value_or(def),
            radius.value_or(METEOR_DEFAULT_RADIUS), _onPlayerKilledByAdmin);
    if (name == "flood")
        return std::make_unique<event::Flood>(
            center.value_or(game::Position(FLOOD_DEFAULT_ORIGIN,
                FLOOD_DEFAULT_ORIGIN)),
            radius.value_or(FLOOD_DEFAULT_WIDTH),
            radius.value_or(FLOOD_DEFAULT_HEIGHT), FLOOD_DEFAULT_DURATION);
    return nullptr;
}

std::unique_ptr<zappy::server::event::Event>
zappy::server::gui::GuiDispatcher::createEventByName(
    std::string_view raw) const
{
    std::istringstream iss{std::string(raw)};
    std::string name;
    int x = 0;
    int y = 0;
    int r = 0;
    std::optional<game::Position> center;
    std::optional<int> radius;

    iss >> name;
    if (static_cast<bool>(iss >> x >> y))
        center = game::Position(x, y);
    if (static_cast<bool>(iss >> r))
        radius = r;
    return makeEvent(name, center, radius);
}

/**
 * @brief Handle adm_event_trigger: spawn the named event on the bus.
 *
 * @param cmd parsed command; rawArgument holds the event name
 * @return std::vector<std::string> "ok" if spawned, "ko" otherwise
 */
std::vector<std::string>
zappy::server::gui::GuiDispatcher::handleAdmEventTrigger(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    std::vector<std::string> response = {"ko"};
    auto event = createEventByName(cmd.rawArgument);

    if (event && _eventBus.spawn(std::move(event)))
        response = {"ok"};
    return response;
}

/**
 * @brief Handle adm_tile_set: overwrite a resource count on a tile.
 *
 * @param cmd parsed command; arguments are x, y, n; rawArgument the resource
 * @return std::vector<std::string> "ok" or "ko" on bad tile/resource
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmTileSet(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int x = cmd.arguments[0];
    const int y = cmd.arguments[1];
    const int n = cmd.arguments[2];
    const bool inBounds = x < _world.width() && y < _world.height();
    const auto type = game::ResourceNameResolver::resolve(cmd.rawArgument);
    std::vector<std::string> response = {"ko"};

    if (inBounds && n >= 0 && type) {
        _world.setTileResource(game::Position(x, y), *type, n);
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_tile_add: add to a resource count on a tile.
 *
 * @param cmd parsed command; arguments are x, y, n; rawArgument the resource
 * @return std::vector<std::string> "ok" or "ko" on bad tile/resource
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmTileAdd(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int x = cmd.arguments[0];
    const int y = cmd.arguments[1];
    const int n = cmd.arguments[2];
    const bool inBounds = x < _world.width() && y < _world.height();
    const auto type = game::ResourceNameResolver::resolve(cmd.rawArgument);
    std::vector<std::string> response = {"ko"};

    if (inBounds && n >= 0 && type) {
        _world.addTileResource(game::Position(x, y), *type, n);
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_player_kill: kill a player and disconnect its AI client.
 *
 * @param cmd parsed command; arguments[0] is the player id
 * @return std::vector<std::string> "ok" or "ko" if unknown/already dead
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmPlayerKill(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int id = cmd.arguments[0];
    std::vector<std::string> response = {"ko"};

    if (_world.hasPlayer(id)
        && _world.player(id).state() != game::PlayerState::DEAD) {
        _world.killPlayer(id);
        if (_onPlayerKilledByAdmin)
            _onPlayerKilledByAdmin(id);
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_player_tp: teleport a player, wrapping the destination.
 *
 * @param cmd parsed command; arguments are id, x, y
 * @return std::vector<std::string> "ok" or "ko" if the player is unknown
 */
std::vector<std::string> zappy::server::gui::GuiDispatcher::handleAdmPlayerTp(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int id = cmd.arguments[0];
    const game::Position dest = game::Position(
        cmd.arguments[1], cmd.arguments[2]).normalized(
            _world.width(), _world.height());
    std::vector<std::string> response = {"ko"};

    if (_world.hasPlayer(id)) {
        _world.movePlayer(id, dest);
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_player_level: set a player's elevation level.
 *
 * @param cmd parsed command; arguments are id, level
 * @return std::vector<std::string> "ok" or "ko" if unknown/invalid level
 */
std::vector<std::string>
zappy::server::gui::GuiDispatcher::handleAdmPlayerLevel(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int id = cmd.arguments[0];
    const int level = cmd.arguments[1];
    const bool validLevel = level >= 1
        && level <= game::ElevationRules::maxLevel();
    std::vector<std::string> response = {"ko"};

    if (_world.hasPlayer(id) && validLevel) {
        _world.setPlayerLevel(id, level);
        response = {"ok"};
    }
    return response;
}

/**
 * @brief Handle adm_player_incant: force-start an incantation on a player.
 *
 * @param cmd parsed command; arguments[0] is the player id
 * @return std::vector<std::string> "ok" or "ko" if it cannot start
 */
std::vector<std::string>
zappy::server::gui::GuiDispatcher::handleAdmPlayerIncant(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int id = cmd.arguments[0];

    if (_world.hasPlayer(id) && _onStartIncantation
        && _onStartIncantation(id))
        return {"ok"};
    return {"ko"};
}

/**
 * @brief Handle adm_player_stop_incant: force-stop a player's incantation.
 *
 * @param cmd parsed command; arguments[0] is the player id
 * @return std::vector<std::string> "ok" or "ko" if none is running
 */
std::vector<std::string>
zappy::server::gui::GuiDispatcher::handleAdmPlayerStopIncant(
    const protocol::gui::ParsedGuiCommand &cmd)
{
    const int id = cmd.arguments[0];

    if (_world.hasPlayer(id) && _onStopIncantation
        && _onStopIncantation(id))
        return {"ok"};
    return {"ko"};
}
