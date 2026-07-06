/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Implementation of the RAII TCP listener
*/

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include "net/Listener.hpp"
#include "net/NetworkError.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>


/**
 * @brief Create a non-blocking TCP listening socket bound to INADDR_ANY.
 *
 * @param port server port (0 lets the OS pick a free port)
 * @param backlog kernel queue size for pending connections
 * @return zappy::posix::FileDescriptor owning the listening socket
 */
static zappy::posix::FileDescriptor makeListeningSocket(
    std::uint16_t port, int backlog)
{
    zappy::posix::FileDescriptor sock(
        ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
    const int yes = 1;

    if (!sock.is_valid())
        throw zappy::net::NetworkError(
            std::string("socket: ") + std::strerror(errno));
    if (::setsockopt(sock.get(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        throw zappy::net::NetworkError(
            std::string("setsockopt: ") + std::strerror(errno));

    const zappy::posix::Address addr =
        zappy::posix::Address::any(port);

    if (::bind(sock.get(), addr.raw_generic(), addr.size()) < 0)
        throw zappy::net::NetworkError(
            std::string("bind: ") + std::strerror(errno));
    if (::listen(sock.get(), backlog) < 0)
        throw zappy::net::NetworkError(
            std::string("listen: ") + std::strerror(errno));
    return sock;
}

/**
 * @brief Read the locally bound port (resolves OS-assigned port when bound on 0).
 *
 * @param sock a bound listening socket
 * @return std::uint16_t the port the kernel actually bound the socket to
 */
static std::uint16_t resolvePort(
    const zappy::posix::FileDescriptor &sock)
{
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);

    if (::getsockname(sock.get(), reinterpret_cast<sockaddr *>(&addr), &len) < 0)
        throw zappy::net::NetworkError(
            std::string("getsockname: ") + std::strerror(errno));
    return ntohs(addr.sin_port);
}

/**
 * @brief Create, bind and listen on @p port; resolve the actually bound port.
 *
 * @param port server port
 * @param backlog kernel queue size for pending connections
 */
zappy::net::Listener::Listener(std::uint16_t port, int backlog)
    : _socket(makeListeningSocket(port, backlog)), _port(resolvePort(_socket))
{
}

/**
 * @brief Raw fd of the listening socket, for poll registration.
 *
 * @return int the listening socket file descriptor
 */
int zappy::net::Listener::fd() const noexcept
{
    return _socket.get();
}

/**
 * @brief Port the socket is actually bound to (useful when constructed with port 0).
 *
 * @return std::uint16_t the bound port
 */
std::uint16_t zappy::net::Listener::port() const noexcept
{
    return _port;
}

/**
 * @brief Accept one pending connection without blocking.
 *
 * @return std::optional<zappy::posix::FileDescriptor> the accepted client
 *         socket, or std::nullopt on EAGAIN/EWOULDBLOCK
 */
std::optional<zappy::posix::FileDescriptor>
zappy::net::Listener::accept_connection() const
{
    const int client = ::accept4(_socket.get(), nullptr, nullptr, SOCK_NONBLOCK);

    if (client < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return std::nullopt;
        throw NetworkError(std::string("accept: ") + std::strerror(errno));
    }
    return zappy::posix::FileDescriptor(client);
}

/**
 * @brief Resolve the peer address of an already-connected client socket.
 *
 * @param client an accepted client socket
 * @return zappy::posix::Address the remote address of the peer
 */
zappy::posix::Address zappy::net::Listener::peer_address(
    const zappy::posix::FileDescriptor &client)
{
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);

    if (::getpeername(client.get(), reinterpret_cast<sockaddr *>(&addr),
            &len) < 0)
        throw NetworkError(std::string("getpeername: ") + std::strerror(errno));
    return zappy::posix::Address(addr);
}
