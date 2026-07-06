/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the GUI admin command set (ZAP-47, bonus)
*/

#include <criterion/criterion.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>

#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/game/World.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/client/ClientState.hpp"
#include "server/config/FeatureFlag.hpp"
#include "server/event/Event.hpp"
#include "server/event/EventBus.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"
#include "posix/FileDescriptor.hpp"

using zappy::server::gui::GuiDispatcher;
using zappy::server::gui::GuiAdminDeps;
using zappy::server::gui::GuiNotifier;
using zappy::server::game::World;
using zappy::server::game::Position;
using zappy::server::game::Orientation;
using zappy::server::game::ResourceType;
using zappy::server::game::PlayerState;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::client::Client;
using zappy::server::client::ClientRegistry;
using zappy::server::client::ClientState;
using zappy::server::config::FeatureFlags;
using zappy::server::config::FeatureFlag;
using zappy::server::event::EventBus;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::SteadyClock;
using zappy::posix::FileDescriptor;

namespace {

using Reply = std::pair<int, std::string>;

struct KillSpyObserver : WorldObserverAdapter {
    bool stateChanged = false;
    bool removed = false;
    bool tileChanged = false;

    void onPlayerStateChanged(int) override { stateChanged = true; }
    void onPlayerRemoved(int) override { removed = true; }
    void onTileChanged(Position) override { tileChanged = true; }
};

struct AdminCommandsFixture {
    World world{10, 10, {"team"}, 5};
    ClientRegistry clients;
    FeatureFlags flags;
    std::vector<Reply> replies;
    std::vector<int> killed;
    int frequency = 100;
    int adminFd;
    int guiFd;
    int playerId;
    SteadyClock clock;
    Scheduler scheduler{clock};
    GuiNotifier notifier{clients, world, [](int) {}};
    EventBus eventBus{world, notifier, scheduler, flags};
    GuiDispatcher disp;

    AdminCommandsFixture()
        : disp(world,
            GuiAdminDeps{clients, flags, "secret",
                [this](int id) { killed.push_back(id); }, eventBus},
            [this](int fd, std::string r) {
                replies.push_back({fd, std::move(r)});
            },
            [this] { return frequency; },
            [this](int f) { frequency = f; })
    {
        flags.enable(FeatureFlag::ADMIN);
        adminFd = addClient(ClientState::GUI_ADMIN);
        guiFd = addClient(ClientState::GUI);
        playerId = world.addPlayer("team", Position(1, 1), Orientation::NORTH);
    }

    int addClient(ClientState state)
    {
        int sv[2];

        socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        close(sv[1]);
        Client &c = clients.add(FileDescriptor{sv[0]});
        c.promote(state);
        return c.fd();
    }

    bool repliesAre(const std::vector<std::string> &expected) const
    {
        if (replies.size() != expected.size())
            return false;
        for (std::size_t i = 0; i < expected.size(); ++i)
            if (replies[i].second != expected[i])
                return false;
        return true;
    }

    bool repliesContain(const std::string &line) const
    {
        return std::any_of(replies.begin(), replies.end(),
            [&line](const Reply &r) { return r.second == line; });
    }
};

} // namespace

Test(gui_admin_commands, non_admin_client_gets_suc)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.guiFd, "adm_flag_list");
    cr_assert(fx.repliesAre({"suc"}));
    fx.replies.clear();
    fx.disp.dispatch(fx.guiFd, "adm_player_kill #1");
    cr_assert(fx.repliesAre({"suc"}));
}

Test(gui_admin_commands, flag_enable_disable_list_roundtrip)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_flag_enable events");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert(fx.flags.isEnabled(FeatureFlag::EVENTS));
    fx.replies.clear();
    fx.disp.dispatch(fx.adminFd, "adm_flag_list");
    cr_assert(fx.repliesContain("adm_flag_list events on"));
    fx.replies.clear();
    fx.disp.dispatch(fx.adminFd, "adm_flag_disable events");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert(!fx.flags.isEnabled(FeatureFlag::EVENTS));
}

Test(gui_admin_commands, biomes_flag_is_boot_only_and_rejected)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_flag_enable biomes");
    cr_assert(fx.repliesAre({"ko"}));
    cr_assert(!fx.flags.isEnabled(FeatureFlag::BIOMES));
    fx.replies.clear();
    fx.disp.dispatch(fx.adminFd, "adm_flag_disable biomes");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, flag_enable_unknown_returns_ko)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_flag_enable bogus");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, event_trigger_returns_ko)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_event_trigger storm");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, tile_set_sets_resource_count)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_tile_set 3 4 5 food");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert_eq(
        fx.world.tileAt(Position(3, 4)).resource(ResourceType::FOOD), 5);
}

Test(gui_admin_commands, tile_set_out_of_bounds_returns_ko)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_tile_set 100 100 5 food");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, tile_set_unknown_resource_returns_ko)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_tile_set 3 4 5 stone");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, tile_add_adds_units)
{
    AdminCommandsFixture fx;

    fx.world.setTileResource(Position(3, 4), ResourceType::FOOD, 2);
    fx.disp.dispatch(fx.adminFd, "adm_tile_add 3 4 3 food");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert_eq(
        fx.world.tileAt(Position(3, 4)).resource(ResourceType::FOOD), 5);
}

Test(gui_admin_commands, player_kill_kills_and_invokes_callback)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_player_kill #1");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert(fx.world.player(1).state() == PlayerState::DEAD);
    cr_assert_eq(fx.killed.size(), 1U);
    cr_assert_eq(fx.killed[0], 1);
}

Test(gui_admin_commands, player_kill_already_dead_returns_ko)
{
    AdminCommandsFixture fx;

    fx.world.killPlayer(1);
    fx.disp.dispatch(fx.adminFd, "adm_player_kill #1");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, player_kill_unknown_id_returns_ko)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_player_kill #999");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, player_tp_teleports_player)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_player_tp #1 7 8");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert(fx.world.player(1).position() == Position(7, 8));
}

Test(gui_admin_commands, player_tp_wraps_coordinates)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_player_tp #1 15 25");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert(fx.world.player(1).position() == Position(5, 5));
}

Test(gui_admin_commands, player_level_sets_level)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_player_level #1 5");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert_eq(fx.world.player(1).level(), 5);
}

Test(gui_admin_commands, player_level_invalid_returns_ko)
{
    AdminCommandsFixture fx;

    fx.disp.dispatch(fx.adminFd, "adm_player_level #1 0");
    cr_assert(fx.repliesAre({"ko"}));
    fx.replies.clear();
    fx.disp.dispatch(fx.adminFd, "adm_player_level #1 9");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, player_kill_fires_observer_events)
{
    AdminCommandsFixture fx;
    KillSpyObserver observer;

    fx.world.addObserver(observer);
    fx.disp.dispatch(fx.adminFd, "adm_player_kill #1");
    cr_assert(observer.stateChanged);
    cr_assert(observer.removed);
}

Test(gui_admin_commands, tile_set_fires_tile_changed_observer)
{
    AdminCommandsFixture fx;
    KillSpyObserver observer;

    fx.world.addObserver(observer);
    fx.disp.dispatch(fx.adminFd, "adm_tile_set 3 4 5 food");
    cr_assert(observer.tileChanged);
}

namespace {

class IdleEvent : public zappy::server::event::Event {
public:
    std::string_view name() const noexcept override { return "idle"; }
    std::string startBroadcast() const override { return "evt_idle_start"; }
    std::string endBroadcast() const override { return "evt_idle_end"; }
    bool applyTick(World &) override { return true; }
    void onEnd(World &) override {}
};

} // namespace

Test(gui_admin_commands, event_trigger_storm_returns_ok_when_events_on)
{
    AdminCommandsFixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.disp.dispatch(fx.adminFd, "adm_event_trigger storm");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert_eq(fx.eventBus.activeEventCount(), 1U);
}

Test(gui_admin_commands, event_trigger_returns_ko_for_unknown_event)
{
    AdminCommandsFixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.disp.dispatch(fx.adminFd, "adm_event_trigger bogus");
    cr_assert(fx.repliesAre({"ko"}));
}

Test(gui_admin_commands, flag_disable_events_cancels_active_events)
{
    AdminCommandsFixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.eventBus.spawn(std::make_unique<IdleEvent>());
    cr_assert_eq(fx.eventBus.activeEventCount(), 1U);
    fx.disp.dispatch(fx.adminFd, "adm_flag_disable events");
    cr_assert(fx.repliesAre({"ok"}));
    cr_assert_eq(fx.eventBus.activeEventCount(), 0U);
    cr_assert(!fx.flags.isEnabled(FeatureFlag::EVENTS));
}

Test(gui_admin_commands, flag_disable_admin_does_not_cancel_events)
{
    AdminCommandsFixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.eventBus.spawn(std::make_unique<IdleEvent>());
    fx.disp.dispatch(fx.adminFd, "adm_flag_disable admin");
    cr_assert_eq(fx.eventBus.activeEventCount(), 1U);
}
