/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for GuiNotifier class
*/

#include <criterion/criterion.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <vector>
#include <cerrno>

#include "server/game/World.hpp"
#include "server/client/ClientRegistry.hpp"
#include "server/gui/GuiNotifier.hpp"

using zappy::server::game::World;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;
using zappy::server::client::Client;
using zappy::server::client::ClientRegistry;
using zappy::server::client::ClientState;
using zappy::server::gui::GuiNotifier;
using zappy::posix::FileDescriptor;

struct NotifierFixture {
    World world{10, 10, {"team1"}, 2};
    ClientRegistry clients;
    std::vector<int> retunedFds;
    std::vector<int> peerFds;
    GuiNotifier notifier{clients, world, [this](int fd) {
        retunedFds.push_back(fd);
    }};

    int addClient(ClientState state)
    {
        int sv[2];
        cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
        fcntl(sv[0], F_SETFL, fcntl(sv[0], F_GETFL) | O_NONBLOCK);
        fcntl(sv[1], F_SETFL, fcntl(sv[1], F_GETFL) | O_NONBLOCK);
        peerFds.push_back(sv[1]);
        Client &c = clients.add(FileDescriptor{sv[0]});
        if (state != ClientState::HANDSHAKE) {
            c.promote(state);
        }
        return sv[1];
    }

    ~NotifierFixture()
    {
        for (int fd : peerFds) {
            close(fd);
        }
    }
};

Test(gui_notifier, test_broadcast_to_gui_clients_only)
{
    NotifierFixture f;
    int guiPeer = f.addClient(ClientState::GUI);
    int adminPeer = f.addClient(ClientState::GUI_ADMIN);
    int aiPeer = f.addClient(ClientState::AI);
    int handshakePeer = f.addClient(ClientState::HANDSHAKE);

    f.notifier.broadcast("test message");

    cr_assert_eq(f.retunedFds.size(), 2U);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "test message\n");

    std::memset(buf, 0, sizeof(buf));
    cr_assert_gt(read(adminPeer, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "test message\n");

    std::memset(buf, 0, sizeof(buf));
    cr_assert_eq(read(aiPeer, buf, sizeof(buf) - 1), -1);
    cr_assert_eq(errno, EAGAIN);

    cr_assert_eq(read(handshakePeer, buf, sizeof(buf) - 1), -1);
    cr_assert_eq(errno, EAGAIN);
}

Test(gui_notifier, test_broadcast_no_gui_clients)
{
    NotifierFixture f;
    int aiPeer = f.addClient(ClientState::AI);
    int handshakePeer = f.addClient(ClientState::HANDSHAKE);

    f.notifier.broadcast("test message");

    cr_assert_eq(f.retunedFds.size(), 0U);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_eq(read(aiPeer, buf, sizeof(buf) - 1), -1);
    cr_assert_eq(errno, EAGAIN);

    cr_assert_eq(read(handshakePeer, buf, sizeof(buf) - 1), -1);
    cr_assert_eq(errno, EAGAIN);
}

Test(gui_notifier, test_broadcast_multiple_gui_clients)
{
    NotifierFixture f;
    int guiPeer1 = f.addClient(ClientState::GUI);
    int guiPeer2 = f.addClient(ClientState::GUI);
    int adminPeer = f.addClient(ClientState::GUI_ADMIN);

    f.notifier.broadcast("msg");

    cr_assert_eq(f.retunedFds.size(), 3U);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_gt(read(guiPeer1, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "msg\n");

    std::memset(buf, 0, sizeof(buf));
    cr_assert_gt(read(guiPeer2, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "msg\n");

    std::memset(buf, 0, sizeof(buf));
    cr_assert_gt(read(adminPeer, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "msg\n");
}

Test(gui_notifier, test_on_tile_changed_fires_bct)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    f.world.setTileResource(Position(0, 0), ResourceType::FOOD, 5);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "bct 0 0 5 0 0 0 0 0 0\n");
}

Test(gui_notifier, test_multiple_mutations_multiple_broadcasts)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    f.world.setTileResource(Position(1, 2), ResourceType::LINEMATE, 3);
    f.world.setTileResource(Position(3, 4), ResourceType::DERAUMERE, 4);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    cr_assert_neq(received.find("bct 1 2 0 3 0 0 0 0 0\n"), std::string::npos);
    cr_assert_neq(received.find("bct 3 4 0 0 4 0 0 0 0\n"), std::string::npos);
}

Test(gui_notifier, test_empty_buffer_state_before_broadcast)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    f.clients.forEach([](Client &c) {
        cr_assert_not(c.buffer().has_pending_write());
    });

    f.world.setTileResource(Position(0, 0), ResourceType::FOOD, 1);

    bool hasWrite = false;
    f.clients.forEach([&hasWrite](Client &c) {
        if (c.buffer().has_pending_write())
            hasWrite = true;
    });
    cr_assert(hasWrite);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "bct 0 0 1 0 0 0 0 0 0\n");
}

Test(gui_notifier, test_on_player_added_broadcasts_pnw)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string expected = "pnw #" + std::to_string(playerId)
        + " 1 2 2 1 team1\n";
    cr_assert_str_eq(buf, expected.c_str());
}

Test(gui_notifier, test_on_player_moved_broadcasts_ppo)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.movePlayer(playerId, Position(2, 2));

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "ppo #" + std::to_string(playerId) + " 2 2 2\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_rotated_broadcasts_ppo)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.rotatePlayer(playerId, zappy::server::game::Orientation::SOUTH);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "ppo #" + std::to_string(playerId) + " 1 2 3\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_level_changed_broadcasts_plv)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.setPlayerLevel(playerId, 3);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "plv #" + std::to_string(playerId) + " 3\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_inventory_changed_broadcasts_pin)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.consumePlayerFood(playerId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pin #" + std::to_string(playerId)
        + " 1 2 9 0 0 0 0 0 0\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_removed_dead_broadcasts_pdi)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.killPlayer(playerId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pdi #" + std::to_string(playerId) + "\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_ejected_broadcasts_pex)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.notifyPlayerEjected(playerId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pex #" + std::to_string(playerId) + "\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_broadcast_broadcasts_pbc)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.notifyBroadcast(playerId, "hello world");

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pbc #" + std::to_string(playerId)
        + " hello world\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_fork_started_broadcasts_pfk)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.notifyForkStarted(playerId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pfk #" + std::to_string(playerId) + "\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_incantation_started_broadcasts_pic)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.notifyIncantationStarted(playerId, 1, {playerId});

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pic 1 2 2 #" + std::to_string(playerId) + "\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_incantation_ended_broadcasts_pie)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.notifyIncantationEnded(playerId, true, 2);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pie 1 2 1\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_dropped_resource_broadcasts_pdr)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.player(playerId).addResource(ResourceType::LINEMATE, 1);
    f.world.dropResourceOnTile(playerId, ResourceType::LINEMATE);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pdr #" + std::to_string(playerId) + " 1\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_player_picked_up_resource_broadcasts_pgt)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    f.world.tileAt(Position(1, 2)).addResource(ResourceType::DERAUMERE, 1);
    f.world.takeResourceFromTile(playerId, ResourceType::DERAUMERE);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "pgt #" + std::to_string(playerId) + " 2\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_egg_added_system_broadcasts_enw)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int eggId = f.world.addEgg("team1", Position(3, 4));

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "enw #" + std::to_string(eggId) + " -1 3 4\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_egg_added_player_broadcasts_enw)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int playerId = f.world.addPlayer(
        "team1", Position(1, 2), zappy::server::game::Orientation::EAST);
    int eggId = f.world.addEgg("team1", Position(1, 2), playerId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "enw #" + std::to_string(eggId) + " #"
        + std::to_string(playerId) + " 1 2\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_egg_removed_hatched_broadcasts_ebo)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int eggId = f.world.addEgg("team1", Position(3, 4));
    f.world.hatchEgg(eggId);
    f.world.removeEgg(eggId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "ebo #" + std::to_string(eggId) + "\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_on_egg_removed_waiting_broadcasts_edi)
{
    NotifierFixture f;
    f.world.addObserver(f.notifier);
    int guiPeer = f.addClient(ClientState::GUI);

    int eggId = f.world.addEgg("team1", Position(3, 4));
    f.world.removeEgg(eggId);

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[256] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    std::string received(buf);
    std::string expected = "edi #" + std::to_string(eggId) + "\n";
    cr_assert_neq(received.find(expected), std::string::npos);
}

Test(gui_notifier, test_victory_detection_sends_seg)
{
    World world{10, 10, {"winning_team"}, 10};
    ClientRegistry clients;
    std::vector<int> retunedFds;

    GuiNotifier notifier{clients, world, [&retunedFds](int fd) {
        retunedFds.push_back(fd);
    }};
    world.addObserver(notifier);

    int sv[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
    fcntl(sv[0], F_SETFL, fcntl(sv[0], F_GETFL) | O_NONBLOCK);
    fcntl(sv[1], F_SETFL, fcntl(sv[1], F_GETFL) | O_NONBLOCK);

    Client &c = clients.add(FileDescriptor{sv[0]});
    c.promote(ClientState::GUI);

    std::vector<int> players;
    for (int i = 0; i < 6; ++i) {
        players.push_back(world.addPlayer("winning_team", Position(i, 0),
            zappy::server::game::Orientation::NORTH));
    }

    for (int i = 0; i < 5; ++i) {
        world.setPlayerLevel(players[i], 8);
    }

    clients.forEach([](Client &cl) { cl.buffer().on_writable(); });
    char dump[4096];
    while (read(sv[1], dump, sizeof(dump)) > 0);

    world.setPlayerLevel(players[5], 8);

    clients.forEach([](Client &cl) { cl.buffer().on_writable(); });

    char buf[1024] = {0};
    cr_assert_gt(read(sv[1], buf, sizeof(buf) - 1), 0);
    std::string received(buf);

    cr_assert_neq(received.find("plv #" + std::to_string(players[5]) + " 8\n"),
        std::string::npos);
    cr_assert_neq(received.find("seg winning_team\n"), std::string::npos);

    std::memset(buf, 0, sizeof(buf));
    int extraPlayer = world.addPlayer("winning_team", Position(7, 0),
        zappy::server::game::Orientation::NORTH);
    world.setPlayerLevel(extraPlayer, 8);
    clients.forEach([](Client &cl) { cl.buffer().on_writable(); });

    cr_assert_gt(read(sv[1], buf, sizeof(buf) - 1), 0);
    received = buf;
    cr_assert_neq(received.find("plv #" + std::to_string(extraPlayer) + " 8\n"),
        std::string::npos);
    cr_assert_eq(received.find("seg winning_team\n"), std::string::npos);

    close(sv[1]);
}

Test(gui_notifier, test_broadcast_message_sends_smg)
{
    NotifierFixture f;
    int guiPeer = f.addClient(ClientState::GUI);

    f.notifier.broadcastMessage("Hello GUI!");

    f.clients.forEach([](Client &c) {
        c.buffer().on_writable();
    });

    char buf[128] = {0};
    cr_assert_gt(read(guiPeer, buf, sizeof(buf) - 1), 0);
    cr_assert_str_eq(buf, "smg Hello GUI!\n");
}
