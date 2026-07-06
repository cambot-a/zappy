/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for Client and ClientRegistry
*/

#include <criterion/criterion.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "server/client/Client.hpp"
#include "server/client/ClientRegistry.hpp"
#include "server/client/ClientState.hpp"
#include "server/client/ClientStateError.hpp"
#include "posix/FileDescriptor.hpp"

static zappy::server::client::Client make_client(int fd)
{
    return zappy::server::client::Client{
        zappy::posix::FileDescriptor{fd}};
}

static int open_pair(int sv[2])
{
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
    close(sv[1]);
    return sv[0];
}

/* Client */

Test(client, default_state_is_handshake)
{
    int sv[2];
    auto c = make_client(open_pair(sv));
    cr_assert_eq(c.state(), zappy::server::client::ClientState::HANDSHAKE);
}

Test(client, promote_to_ai_sets_state_and_payload)
{
    int sv[2];
    auto c = make_client(open_pair(sv));
    c.promote(zappy::server::client::ClientState::AI);
    cr_assert_eq(c.state(), zappy::server::client::ClientState::AI);
    cr_assert_eq(c.aiData().playerId, 0);
}

Test(client, promote_to_gui_sets_state_and_payload)
{
    int sv[2];
    auto c = make_client(open_pair(sv));
    c.promote(zappy::server::client::ClientState::GUI);
    cr_assert_eq(c.state(), zappy::server::client::ClientState::GUI);
}

Test(client, promote_to_gui_admin_sets_state_and_payload)
{
    int sv[2];
    auto c = make_client(open_pair(sv));
    c.promote(zappy::server::client::ClientState::GUI_ADMIN);
    cr_assert_eq(c.state(), zappy::server::client::ClientState::GUI_ADMIN);
}

Test(client, aidata_throws_when_not_ai)
{
    int sv[2];
    auto c = make_client(open_pair(sv));
    cr_assert_throw((void)c.aiData(), zappy::server::client::ClientStateError);
}

Test(client, promote_twice_throws)
{
    int sv[2];
    auto c = make_client(open_pair(sv));
    c.promote(zappy::server::client::ClientState::AI);
    cr_assert_throw(
        c.promote(zappy::server::client::ClientState::GUI),
        zappy::server::client::ClientStateError);
}

/* ClientRegistry */

Test(client_registry, add_returns_handshake_client)
{
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    close(sv[1]);
    zappy::server::client::ClientRegistry reg;
    zappy::server::client::Client &c =
        reg.add(zappy::posix::FileDescriptor{sv[0]});
    cr_assert_eq(c.state(), zappy::server::client::ClientState::HANDSHAKE);
}

Test(client_registry, contains_true_after_add_false_after_remove)
{
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    close(sv[1]);
    zappy::server::client::ClientRegistry reg;
    const int fd = sv[0];
    reg.add(zappy::posix::FileDescriptor{fd});
    cr_assert(reg.contains(fd));
    reg.remove(fd);
    cr_assert_not(reg.contains(fd));
}

Test(client_registry, get_unknown_fd_throws)
{
    zappy::server::client::ClientRegistry reg;
    cr_assert_throw((void)reg.get(42), zappy::server::client::ClientStateError);
}

Test(client_registry, double_remove_is_safe)
{
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    close(sv[1]);
    zappy::server::client::ClientRegistry reg;
    const int fd = sv[0];
    reg.add(zappy::posix::FileDescriptor{fd});
    reg.remove(fd);
    reg.remove(fd);
    cr_assert_eq(reg.size(), 0U);
}

Test(client_registry, count_in_state_mixed)
{
    int sva[2], svb[2], svc[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sva);
    close(sva[1]);
    socketpair(AF_UNIX, SOCK_STREAM, 0, svb);
    close(svb[1]);
    socketpair(AF_UNIX, SOCK_STREAM, 0, svc);
    close(svc[1]);

    zappy::server::client::ClientRegistry reg;
    reg.add(zappy::posix::FileDescriptor{sva[0]});
    reg.add(zappy::posix::FileDescriptor{svb[0]})
        .promote(zappy::server::client::ClientState::AI);
    reg.add(zappy::posix::FileDescriptor{svc[0]})
        .promote(zappy::server::client::ClientState::GUI);

    cr_assert_eq(reg.countInState(zappy::server::client::ClientState::HANDSHAKE), 1U);
    cr_assert_eq(reg.countInState(zappy::server::client::ClientState::AI), 1U);
    cr_assert_eq(reg.countInState(zappy::server::client::ClientState::GUI), 1U);
}

Test(client_registry, for_each_in_state_visits_only_matching)
{
    int sva[2], svb[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sva);
    close(sva[1]);
    socketpair(AF_UNIX, SOCK_STREAM, 0, svb);
    close(svb[1]);

    zappy::server::client::ClientRegistry reg;
    reg.add(zappy::posix::FileDescriptor{sva[0]});
    reg.add(zappy::posix::FileDescriptor{svb[0]})
        .promote(zappy::server::client::ClientState::AI);

    std::size_t visited = 0;
    reg.forEachInState(zappy::server::client::ClientState::AI,
        [&visited](zappy::server::client::Client &c) {
            cr_assert_eq(c.state(), zappy::server::client::ClientState::AI);
            ++visited;
        });
    cr_assert_eq(visited, 1U);
}

Test(client_registry, for_each_throwing_callback_leaves_registry_intact)
{
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    close(sv[1]);
    zappy::server::client::ClientRegistry reg;
    reg.add(zappy::posix::FileDescriptor{sv[0]});

    bool threw = false;
    try {
        reg.forEach([](zappy::server::client::Client &) {
            throw std::runtime_error("test error");
        });
    } catch (const std::runtime_error &) {
        threw = true;
    }
    cr_assert(threw);
    cr_assert_eq(reg.size(), 1U);
}
