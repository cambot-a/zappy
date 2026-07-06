/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GUI command dispatcher routing lines to stubs
*/

#ifndef SERVER_GUI_GUIDISPATCHER_HPP_
    #define SERVER_GUI_GUIDISPATCHER_HPP_

    #include <array>
    #include <functional>
    #include <memory>
    #include <optional>
    #include <string>
    #include <string_view>
    #include <vector>

    #include "server/game/World.hpp"
    #include "server/gui/GuiLineBuilder.hpp"
    #include "server/client/ClientRegistry.hpp"
    #include "server/config/FeatureFlags.hpp"
    #include "protocol/gui/GuiCommandParser.hpp"

namespace zappy::server::event {
class Event;
class EventBus;
} // namespace zappy::server::event

namespace zappy::server::gui {

/**
 * @brief Bundle of admin dependencies for the dispatcher.
 *
 * Grouped into one parameter so the constructor stays within the argument
 * limit while still owning the registry, feature flags, admin password, the
 * kill callback and the event bus used by the admin command handlers.
 */
struct GuiAdminDeps {
    client::ClientRegistry &clients;
    config::FeatureFlags &featureFlags;
    std::string adminPassword;
    std::function<void(int)> onPlayerKilledByAdmin;
    event::EventBus &eventBus;
    std::function<bool(int)> onStartIncantation = nullptr;
    std::function<bool(int)> onStopIncantation = nullptr;
};

/**
 * @brief Parses incoming GUI command lines and
 * executes the corresponding stub.
 */
class GuiDispatcher {
public:
    /**
     * @brief Wire the dispatcher to the world, admin deps and callbacks.
     *
     * @param world game world details
     * @param adminDeps client registry, feature flags and admin password
     * @param onResponse callback delivering a response message to a client fd
     * @param getFrequency callback to query the current frequency
     * @param setFrequency callback to update the time frequency
     */
    GuiDispatcher(game::World &world, GuiAdminDeps adminDeps,
        std::function<void(int fd, std::string)> onResponse,
        std::function<int()> getFrequency,
        std::function<void(int)> setFrequency);

    GuiDispatcher(const GuiDispatcher &) = delete;
    GuiDispatcher &operator=(const GuiDispatcher &) = delete;
    GuiDispatcher(GuiDispatcher &&) = delete;
    GuiDispatcher &operator=(GuiDispatcher &&) = delete;

    /**
     * @brief Parse and execute the GUI command line for @p fd.
     *
     * @param fd client file descriptor
     * @param line raw command line
     */
    void dispatch(int fd, std::string_view line);

    /**
     * @brief Send the complete initial state sync to a promoted GUI client.
     *
     * @param fd client file descriptor
     * @param frequency current server clock frequency
     */
    void sendInitialSync(int fd, int frequency);

private:
    using Handler =
        std::vector<std::string> (GuiDispatcher::*)(const std::vector<int> &);
    using AdminHandler = std::vector<std::string> (GuiDispatcher::*)(
        const protocol::gui::ParsedGuiCommand &);

    game::World &_world;
    std::function<void(int, std::string)> _onResponse;
    GuiLineBuilder _lineBuilder;
    std::function<int()> _getFrequency;
    std::function<void(int)> _setFrequency;
    client::ClientRegistry &_clients;
    config::FeatureFlags &_featureFlags;
    std::string _adminPassword;
    std::function<void(int)> _onPlayerKilledByAdmin;
    event::EventBus &_eventBus;
    std::function<bool(int)> _onStartIncantation;
    std::function<bool(int)> _onStopIncantation;

    void sendError(int fd, protocol::gui::GuiParseError err);

    static constexpr int STORM_DEFAULT_RADIUS = 3;
    static constexpr int STORM_DEFAULT_PUSH_INTERVAL = 2;
    static constexpr int STORM_DEFAULT_DURATION = 10;

    static constexpr int FLOOD_DEFAULT_ORIGIN = 1;
    static constexpr int FLOOD_DEFAULT_WIDTH = 3;
    static constexpr int FLOOD_DEFAULT_HEIGHT = 3;
    static constexpr int FLOOD_DEFAULT_DURATION = 8;

    static constexpr int METEOR_DEFAULT_RADIUS = 1;

    /**
     * @brief Parse "<name> [X Y [R]]" and instantiate the matching event.
     *
     * Position and radius are optional; when absent the per-event defaults
     * are used. Unknown names yield nullptr.
     *
     * @param raw the raw argument string (name plus optional X Y R)
     * @return std::unique_ptr<event::Event> the event, or nullptr if unknown
     */
    [[nodiscard]] std::unique_ptr<event::Event> createEventByName(
        std::string_view raw) const;

    /**
     * @brief Build the named event with optional centre and radius overrides.
     *
     * @param name requested event name
     * @param center optional zone centre (per-event default when empty)
     * @param radius optional zone radius (per-event default when empty)
     * @return std::unique_ptr<event::Event> the event, or nullptr if unknown
     */
    [[nodiscard]] std::unique_ptr<event::Event> makeEvent(
        std::string_view name, std::optional<game::Position> center,
        std::optional<int> radius) const;

    /**
     * @brief Whether @p kind is an admin-scoped (adm_*) command.
     *
     * @param kind the command kind
     * @return bool true if kind lies in the contiguous ADM_* range
     */
    [[nodiscard]] static constexpr bool isAdminCommand(
        protocol::gui::GuiCommandKind kind) noexcept
    {
        return kind >= protocol::gui::GuiCommandKind::ADM_FLAG_ENABLE
            && kind <= protocol::gui::GuiCommandKind::ADM_PLAYER_STOP_INCANT;
    }

    /**
     * @brief Route a parsed command to admin, scoped or standard handling.
     *
     * @param fd client file descriptor
     * @param cmd the parsed command
     * @return std::vector<std::string> the response lines
     */
    [[nodiscard]] std::vector<std::string> routeCommand(
        int fd, const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief Execute an adm_* command, gated on the client being GUI_ADMIN.
     *
     * Non-admin clients receive "suc" so admin commands stay invisible.
     *
     * @param fd client file descriptor
     * @param cmd the parsed adm_* command
     * @return std::vector<std::string> the response lines
     */
    [[nodiscard]] std::vector<std::string> handleAdminScopedCommand(
        int fd, const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief Run a standard integer-argument command through its handler.
     *
     * @param cmd the parsed command (never ADMIN)
     * @return std::vector<std::string> the response lines
     */
    [[nodiscard]] std::vector<std::string> dispatchStandard(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief Handle the admin authentication command for @p fd.
     *
     * Returns "suc" when the ADMIN flag is off (hides the command), "sbp" on
     * an empty password, "ok" on a correct password (promoting the client),
     * and "ko" otherwise. Never logs nor echoes the password.
     *
     * @param fd client file descriptor
     * @param cmd the parsed admin command (rawArgument holds the password)
     * @return std::vector<std::string> the single response line
     */
    [[nodiscard]] std::vector<std::string> handleAdmin(
        int fd, const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief Promote the GUI client at @p fd to GUI_ADMIN if eligible.
     *
     * @param fd client file descriptor
     * @return bool true if the client was GUI/GUI_ADMIN and is now admin
     */
    [[nodiscard]] bool promoteClientAdmin(int fd);

    /**
     * @brief Build the "msz <width> <height>" line for the current world.
     *
     * @return std::string the formatted map-size line
     */
    [[nodiscard]] std::string buildMszLine() const;


    void sendSyncMapSize(int fd);
    void sendSyncTiles(int fd);
    void sendSyncTile(int fd, int x, int y);
    void sendSyncTeams(int fd);
    void sendSyncPlayers(int fd);
    void sendSyncPlayer(int fd, const game::Player &player);
    void sendSyncPlayerInventory(int fd, const game::Player &player);
    void sendSyncEggs(int fd);
    void sendSyncTime(int fd, int frequency);
    void sendSyncBiomes(int fd);
    void sendSyncBiomeTile(int fd, int x, int y);

    [[nodiscard]] std::vector<std::string> handleMsz(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handleBct(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handleMct(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handleTna(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handlePpo(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handlePlv(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handlePin(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handleSgt(
        const std::vector<int> &args);
    [[nodiscard]] std::vector<std::string> handleSst(
        const std::vector<int> &args);

    /**
     * @brief Handle the ppf command: aggregated player profile snapshot.
     *
     * Gated on the PROFILE feature flag; behaves as an unknown command
     * ("suc") when the flag is off, "sbp" for an unknown player id.
     *
     * @param args command arguments: the player id
     * @return std::vector<std::string> the ppf line, "suc" or "sbp"
     */
    [[nodiscard]] std::vector<std::string> handlePpf(
        const std::vector<int> &args);

    /**
     * @brief adm_flag_enable: turn a feature flag on by name.
     *
     * @param cmd parsed command; rawArgument holds the flag name
     * @return std::vector<std::string> "ok" or "ko" if the name is unknown
     */
    [[nodiscard]] std::vector<std::string> handleAdmFlagEnable(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_flag_disable: turn a feature flag off by name.
     *
     * @param cmd parsed command; rawArgument holds the flag name
     * @return std::vector<std::string> "ok" or "ko" if the name is unknown
     */
    [[nodiscard]] std::vector<std::string> handleAdmFlagDisable(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_flag_list: report every flag and its on/off state.
     *
     * @param cmd parsed command (unused)
     * @return std::vector<std::string> one "adm_flag_list <name> <on|off>" line
     */
    [[nodiscard]] std::vector<std::string> handleAdmFlagList(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_event_trigger: stub until the event bus (ZAP-48) exists.
     *
     * @param cmd parsed command (unused)
     * @return std::vector<std::string> always "ko"
     */
    [[nodiscard]] std::vector<std::string> handleAdmEventTrigger(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_tile_set: overwrite a resource count on a tile.
     *
     * @param cmd parsed command; arguments x, y, n; rawArgument the resource
     * @return std::vector<std::string> "ok" or "ko" on bad tile/resource
     */
    [[nodiscard]] std::vector<std::string> handleAdmTileSet(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_tile_add: add to a resource count on a tile.
     *
     * @param cmd parsed command; arguments x, y, n; rawArgument the resource
     * @return std::vector<std::string> "ok" or "ko" on bad tile/resource
     */
    [[nodiscard]] std::vector<std::string> handleAdmTileAdd(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_player_kill: kill a player and disconnect its AI client.
     *
     * @param cmd parsed command; arguments[0] is the player id
     * @return std::vector<std::string> "ok" or "ko" if unknown/already dead
     */
    [[nodiscard]] std::vector<std::string> handleAdmPlayerKill(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_player_tp: teleport a player, wrapping the destination.
     *
     * @param cmd parsed command; arguments are id, x, y
     * @return std::vector<std::string> "ok" or "ko" if the player is unknown
     */
    [[nodiscard]] std::vector<std::string> handleAdmPlayerTp(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_player_level: set a player's elevation level.
     *
     * @param cmd parsed command; arguments are id, level
     * @return std::vector<std::string> "ok" or "ko" if unknown/invalid level
     */
    [[nodiscard]] std::vector<std::string> handleAdmPlayerLevel(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_player_incant: force-start an incantation on a player.
     *
     * @param cmd parsed command; arguments[0] is the player id
     * @return std::vector<std::string> "ok" or "ko" if it cannot start
     */
    [[nodiscard]] std::vector<std::string> handleAdmPlayerIncant(
        const protocol::gui::ParsedGuiCommand &cmd);

    /**
     * @brief adm_player_stop_incant: force-stop a player's incantation.
     *
     * @param cmd parsed command; arguments[0] is the player id
     * @return std::vector<std::string> "ok" or "ko" if none is running
     */
    [[nodiscard]] std::vector<std::string> handleAdmPlayerStopIncant(
        const protocol::gui::ParsedGuiCommand &cmd);
};

} // namespace zappy::server::gui

#endif /* !SERVER_GUI_GUIDISPATCHER_HPP_ */
