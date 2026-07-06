/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** End-to-end tests for EventBus + Storm + GuiNotifier (ZAP-49, bonus)
*/

#include <criterion/criterion.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "server/event/EventBus.hpp"
#include "server/event/Storm.hpp"
#include "server/config/FeatureFlag.hpp"
#include "server/config/FeatureFlags.hpp"
#include "server/client/ClientRegistry.hpp"
#include "server/game/World.hpp"
#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::event::EventBus;
using zappy::server::event::Storm;
using zappy::server::config::FeatureFlag;
using zappy::server::config::FeatureFlags;
using zappy::server::client::Client;
using zappy::server::client::ClientRegistry;
using zappy::server::client::ClientState;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::gui::GuiAdminDeps;
using zappy::server::gui::GuiDispatcher;
using zappy::server::gui::GuiNotifier;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;
using zappy::posix::FileDescriptor;

namespace {

class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

std::size_t countOccurrences(const std::string &haystack,
    const std::string &needle)
{
    std::size_t count = 0;
    std::size_t pos = haystack.find(needle);

    while (pos != std::string::npos) {
        count++;
        pos = haystack.find(needle, pos + needle.size());
    }
    return count;
}

struct StormBusFixture {
    World world{10, 10, {"team"}, 8};
    FeatureFlags flags;
    ClientRegistry clients;
    std::vector<int> peerFds;
    GuiNotifier notifier{clients, world, [](int) {}};
    FakeClock clock;
    Scheduler scheduler{clock};
    EventBus bus{world, notifier, scheduler, flags};

    StormBusFixture() { world.addObserver(notifier); }

    int addClient(ClientState state)
    {
        int sv[2];

        socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        fcntl(sv[1], F_SETFL, fcntl(sv[1], F_GETFL) | O_NONBLOCK);
        peerFds.push_back(sv[1]);
        Client &c = clients.add(FileDescriptor{sv[0]});
        c.promote(state);
        return c.fd();
    }

    int lastPeer() const { return peerFds.back(); }

    void tick()
    {
        clock.advance(Duration(1000));
        scheduler.tick();
    }

    std::string drain(int peer)
    {
        char buf[2048] = {0};

        clients.forEach([](Client &c) { c.buffer().on_writable(); });
        const ssize_t n = read(peer, buf, sizeof(buf) - 1);
        return n > 0 ? std::string(buf, static_cast<std::size_t>(n)) : "";
    }

    ~StormBusFixture()
    {
        for (int fd : peerFds)
            close(fd);
    }
};

std::unique_ptr<Storm> makeStorm(int duration)
{
    return std::make_unique<Storm>(Position(5, 5), 3, Orientation::EAST,
        1, duration);
}

} // namespace

Test(event_bus_storm, spawn_broadcasts_start)
{
    StormBusFixture fx;
    fx.addClient(ClientState::GUI);
    const int peer = fx.lastPeer();

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    fx.bus.spawn(makeStorm(5));
    cr_assert(fx.drain(peer).find("evt_storm_start 5 5 3 2")
        != std::string::npos);
}

Test(event_bus_storm, expires_after_duration_broadcasts_end)
{
    StormBusFixture fx;
    fx.addClient(ClientState::GUI);
    const int peer = fx.lastPeer();

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    fx.bus.spawn(makeStorm(3));
    for (int i = 0; i < 4; i++)
        fx.tick();
    const std::string out = fx.drain(peer);
    cr_assert(out.find("evt_storm_end") != std::string::npos);
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
}

Test(event_bus_storm, tick_broadcast_fires_each_tick)
{
    StormBusFixture fx;
    fx.addClient(ClientState::GUI);
    const int peer = fx.lastPeer();

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    fx.bus.spawn(makeStorm(3));
    fx.tick();
    cr_assert_eq(countOccurrences(fx.drain(peer), "evt_storm_tick"), 1U);
    fx.tick();
    fx.tick();
    cr_assert_eq(countOccurrences(fx.drain(peer), "evt_storm_tick"), 2U);
}

Test(event_bus_storm, pushed_player_triggers_ppo_broadcast)
{
    StormBusFixture fx;
    fx.addClient(ClientState::GUI);
    const int peer = fx.lastPeer();

    fx.world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    fx.bus.spawn(makeStorm(3));
    fx.tick();
    const std::string out = fx.drain(peer);
    cr_assert(out.find("evt_storm_tick") != std::string::npos);
    cr_assert(out.find("ppo") != std::string::npos);
}

Test(event_bus_storm, admin_trigger_spawns_storm)
{
    StormBusFixture fx;
    std::vector<std::string> replies;
    GuiDispatcher disp(fx.world,
        GuiAdminDeps{fx.clients, fx.flags, "secret", [](int) {}, fx.bus},
        [&replies](int, std::string r) { replies.push_back(std::move(r)); },
        [] { return 100; }, [](int) {});

    fx.flags.enable(FeatureFlag::ADMIN);
    fx.flags.enable(FeatureFlag::EVENTS);
    const int adminFd = fx.addClient(ClientState::GUI_ADMIN);
    disp.dispatch(adminFd, "adm_event_trigger storm");
    cr_assert(!replies.empty());
    cr_assert_str_eq(replies.back().c_str(), "ok");
    cr_assert_eq(fx.bus.activeEventCount(), 1U);
}
