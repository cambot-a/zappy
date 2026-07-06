/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for HandshakeHandler
*/

#include <criterion/criterion.h>
#include <fcntl.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "server/cli/ServerConfig.hpp"
#include "server/client/Client.hpp"
#include "server/client/ClientState.hpp"
#include "server/game/FoodScheduler.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/handshake/HandshakeHandler.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"
#include "posix/FileDescriptor.hpp"

using zappy::server::cli::ServerConfig;
using zappy::server::client::Client;
using zappy::server::client::ClientState;
using zappy::server::game::FoodScheduler;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::handshake::HandshakeHandler;
using zappy::server::handshake::HandshakeResult;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::SteadyClock;
using zappy::posix::FileDescriptor;

/* helpers */

static ServerConfig make_config(int slots = 2)
{
    return ServerConfig(4242, 10, 20, {"alpha", "beta"}, slots, 100);
}

static Client make_client(int &out_peer)
{
    int sv[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
    fcntl(sv[0], F_SETFL, fcntl(sv[0], F_GETFL) | O_NONBLOCK);
    fcntl(sv[1], F_SETFL, fcntl(sv[1], F_GETFL) | O_NONBLOCK);
    out_peer = sv[1];
    return Client{FileDescriptor{sv[0]}};
}

static std::string drain_sent(Client &client, int peer_fd)
{
    client.buffer().on_writable();
    char buf[256] = {};
    const auto n = read(peer_fd, buf, sizeof(buf) - 1);
    return (n > 0) ? std::string(buf, static_cast<std::size_t>(n)) : std::string{};
}

static void seed_eggs(World &world, const std::string &team, int count)
{
    for (int i = 0; i < count; ++i)
        world.addEgg(team, Position(i, 0));
}

/**
 * @brief Records the ordered egg lifecycle events the world emits.
 */
class EggObserver : public WorldObserverAdapter {
public:
    std::vector<std::string> events;

    void onEggHatched(int id) override
    {
        events.push_back("hatch:" + std::to_string(id));
    }

    void onEggRemoved(int id) override
    {
        events.push_back("remove:" + std::to_string(id));
    }
};

/* HandshakeHandler */

Test(handshake_handler, graphic_promotes_to_gui)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "GRAPHIC");
    cr_assert_eq(res, HandshakeResult::PROMOTED);
    cr_assert_eq(client.state(), ClientState::GUI);
    close(peer);
}

Test(handshake_handler, graphic_queues_no_extra_bytes)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    (void)h.handle(client, "GRAPHIC");
    const std::string sent = drain_sent(client, peer);
    cr_assert(sent.empty());
    close(peer);
}

Test(handshake_handler, valid_team_with_egg_promotes_to_ai)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    seed_eggs(world, "alpha", 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "alpha");
    cr_assert_eq(res, HandshakeResult::PROMOTED);
    cr_assert_eq(client.state(), ClientState::AI);
    cr_assert_eq(client.aiData().teamName, "alpha");
    close(peer);
}

Test(handshake_handler, valid_team_queues_remaining_eggs_and_dimensions)
{
    auto config = make_config(3);
    World world(10, 20, {"alpha"}, 3);
    seed_eggs(world, "alpha", 3);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    (void)h.handle(client, "alpha");
    const std::string sent = drain_sent(client, peer);
    cr_assert_str_eq(sent.c_str(), "2\n10 20\n");
    close(peer);
}

Test(handshake_handler, promotion_consumes_one_waiting_egg)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    seed_eggs(world, "alpha", 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    (void)h.handle(client, "alpha");
    cr_assert_eq(world.waitingEggCount("alpha"), 1);
    cr_assert(world.hasPlayer(client.aiData().playerId));
    close(peer);
}

Test(handshake_handler, promotion_hatches_then_removes_the_egg)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    const int eggId = world.addEgg("alpha", Position(4, 5));
    EggObserver obs;
    world.addObserver(obs);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    (void)h.handle(client, "alpha");
    cr_assert_eq(obs.events.size(), 2U);
    cr_assert_str_eq(obs.events[0].c_str(),
        ("hatch:" + std::to_string(eggId)).c_str());
    cr_assert_str_eq(obs.events[1].c_str(),
        ("remove:" + std::to_string(eggId)).c_str());
    close(peer);
}

Test(handshake_handler, unknown_team_drops_with_ko)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    seed_eggs(world, "alpha", 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "ghost");
    cr_assert_eq(res, HandshakeResult::DROP);
    const std::string sent = drain_sent(client, peer);
    cr_assert_str_eq(sent.c_str(), "ko\n");
    close(peer);
}

Test(handshake_handler, no_waiting_egg_drops_with_ko)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "alpha");
    cr_assert_eq(res, HandshakeResult::DROP);
    const std::string sent = drain_sent(client, peer);
    cr_assert_str_eq(sent.c_str(), "ko\n");
    close(peer);
}

Test(handshake_handler, egg_exhaustion_drops_third_client)
{
    auto config = make_config(1);
    World world(10, 20, {"alpha"}, 1);
    seed_eggs(world, "alpha", 1);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);

    int peer1 = -1;
    auto client1 = make_client(peer1);
    (void)h.handle(client1, "alpha");
    close(peer1);

    int peer2 = -1;
    auto client2 = make_client(peer2);
    const auto res = h.handle(client2, "alpha");
    cr_assert_eq(res, HandshakeResult::DROP);
    const std::string sent = drain_sent(client2, peer2);
    cr_assert_str_eq(sent.c_str(), "ko\n");
    close(peer2);
}

Test(handshake_handler, empty_line_drops_with_ko)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "");
    cr_assert_eq(res, HandshakeResult::DROP);
    const std::string sent = drain_sent(client, peer);
    cr_assert_str_eq(sent.c_str(), "ko\n");
    close(peer);
}

Test(handshake_handler, whitespace_only_line_drops_with_ko)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "   \t");
    cr_assert_eq(res, HandshakeResult::DROP);
    close(peer);
}

Test(handshake_handler, leading_trailing_whitespace_tolerated_for_graphic)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, "  GRAPHIC  ");
    cr_assert_eq(res, HandshakeResult::PROMOTED);
    cr_assert_eq(client.state(), ClientState::GUI);
    close(peer);
}

Test(handshake_handler, leading_trailing_whitespace_tolerated_for_team)
{
    auto config = make_config();
    World world(10, 20, {"alpha"}, 2);
    seed_eggs(world, "alpha", 2);
    SteadyClock clock;
    Scheduler sched(clock);
    FoodScheduler food(world, sched, 100, [](int) {});
    HandshakeHandler h(config, world, food);
    int peer = -1;
    auto client = make_client(peer);

    const auto res = h.handle(client, " alpha ");
    cr_assert_eq(res, HandshakeResult::PROMOTED);
    cr_assert_eq(client.state(), ClientState::AI);
    close(peer);
}
