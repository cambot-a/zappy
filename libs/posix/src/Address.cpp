/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Implementation of the sockaddr_in value wrapper
*/

#include "posix/Address.hpp"

#include <arpa/inet.h>

#include <array>
#include <cstdint>
#include <stdexcept>

/**
 * @brief Zero-initialised IPv4 sockaddr_in bound to @p port (host order).
 *
 * @param port port in host byte order
 * @return sockaddr_in initialised with AF_INET and the port in network order
 */
static sockaddr_in makeBase(std::uint16_t port) noexcept
{
    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    return addr;
}

/**
 * @brief Build from a dotted IPv4 string; validates the IP.
 *
 * @param ip dotted IPv4 address (e.g. "127.0.0.1")
 * @param port port in host byte order
 * @throws std::invalid_argument if @p ip is not a valid IPv4 address
 */
zappy::posix::Address::Address(const std::string &ip,
    std::uint16_t port): _addr(makeBase(port))
{
    if (inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr) != 1)
        throw std::invalid_argument("invalid IPv4 address: " + ip);
}

/**
 * @brief Build from an existing sockaddr_in.
 *
 * @param raw existing sockaddr_in to wrap
 */
zappy::posix::Address::Address(const sockaddr_in &raw) noexcept
    : _addr(raw)
{
}

/**
 * @brief Build a wildcard address (INADDR_ANY) on @p port.
 *
 * @param port port in host byte order
 * @return zappy::posix::Address bound to INADDR_ANY:port
 */
zappy::posix::Address
zappy::posix::Address::any(std::uint16_t port) noexcept
{
    sockaddr_in addr = makeBase(port);

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return Address(addr);
}

/**
 * @brief Return the dotted IPv4 string.
 *
 * @return std::string the IPv4 address in dotted decimal notation
 */
std::string zappy::posix::Address::ip() const
{
    std::array<char, INET_ADDRSTRLEN> buffer{};

    inet_ntop(AF_INET, &_addr.sin_addr, buffer.data(), buffer.size());
    return std::string(buffer.data());
}

/**
 * @brief Return the port in host byte order.
 *
 * @return std::uint16_t the port
 */
std::uint16_t zappy::posix::Address::port() const noexcept
{
    return ntohs(_addr.sin_port);
}

/**
 * @brief Return the underlying sockaddr_in.
 *
 * @return const sockaddr_in& reference to the wrapped sockaddr_in
 */
const sockaddr_in &zappy::posix::Address::raw() const noexcept
{
    return _addr;
}

/**
 * @brief Return the address as a generic sockaddr.
 *
 * @return const sockaddr* pointer to the wrapped address as sockaddr
 */
const sockaddr *zappy::posix::Address::raw_generic() const noexcept
{
    return reinterpret_cast<const sockaddr *>(&_addr);
}

/**
 * @brief Return the size of the underlying sockaddr_in.
 *
 * @return socklen_t size of sockaddr_in in bytes
 */
socklen_t zappy::posix::Address::size() const noexcept
{
    return static_cast<socklen_t>(sizeof(_addr));
}
