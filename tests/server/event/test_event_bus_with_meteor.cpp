/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** End-to-end tests for EventBus + Meteor + GuiNotifier (ZAP-51, bonus)
*/

#include <criterion/criterion.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "server/event/EventBus.hpp"
#include "server/event/Meteor.hpp"
#include "server/config/FeatureFlag.hpp"
#include "server/config/FeatureFlags.hpp"
#include "server/client/ClientRegistry.hpp"
#include "server/game/World.hpp"
#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::event::EventBus;
using zappy::server::event::Meteor;
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

struct MeteorBusFixture {
    World world{10, 10, {"team"}, 8};
    FeatureFlags flags;
    ClientRegistry clients;
    std::vector<int> peerFds;
    std::vector<int> killed;
    GuiNotifier notifier{clients, world, [](int) {}};
    FakeClock clock;
    Scheduler scheduler{clock};
    EventBus bus{world, notifier, scheduler, flags};

    MeteorBusFixture() { world.addObserver(notifier); }

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

    std::unique_ptr<Meteor> makeMeteor(Position center, int radius)
    {
        return std::make_unique<Meteor>(center, radius,
            [this](int id) { killed.push_back(id); });
    }

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

    ~MeteorBusFixture()
    {
        for (int fd : peerFds)
            close(fd);
    }
};

} // namespace

Test(event_bus_meteor, spawn_broadcasts_impact)
{
    MeteorBusFixture fx;
    fx.addClient(ClientState::GUI);
    const int peer = fx.lastPeer();

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    fx.bus.spawn(fx.makeMeteor(Position(5, 5), 1));
    cr_assert(fx.drain(peer).find("evt_meteor_impact 5 5 1")
        != std::string::npos);
}

Test(event_bus_meteor, expires_after_one_tick_broadcasts_end)
{
    MeteorBusFixture fx;
    fx.addClient(ClientState::GUI);
    const int peer = fx.lastPeer();

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    fx.bus.spawn(fx.makeMeteor(Position(5, 5), 1));
    fx.tick();
    cr_assert(fx.drain(peer).find("evt_meteor_end") != std::string::npos);
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
}

Test(event_bus_meteor, admin_trigger_spawns_and_kills_center_player)
{
    MeteorBusFixture fx;
    std::vector<std::string> replies;
    GuiDispatcher disp(fx.world,
        GuiAdminDeps{fx.clients, fx.flags, "secret",
            [&fx](int id) { fx.killed.push_back(id); }, fx.bus},
        [&replies](int, std::string r) { replies.push_back(std::move(r)); },
        [] { return 100; }, [](int) {});
    const int id = fx.world.addPlayer("team", Position(5, 5),
        Orientation::NORTH);

    fx.flags.enable(FeatureFlag::ADMIN);
    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.start();
    const int adminFd = fx.addClient(ClientState::GUI_ADMIN);
    disp.dispatch(adminFd, "adm_event_trigger meteor");
    cr_assert_str_eq(replies.back().c_str(), "ok");
    cr_assert_eq(fx.bus.activeEventCount(), 1U);
    fx.tick();
    cr_assert_eq(fx.killed.size(), 1U);
    cr_assert_eq(fx.killed.front(), id);
}
