/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Criterion tests for the Address sockaddr_in wrapper
*/

#include <criterion/criterion.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdexcept>

#include "posix/Address.hpp"

Test(address, builds_from_ip_and_port)
{
    zappy::posix::Address addr("127.0.0.1", 4242);

    cr_assert_str_eq(addr.ip().c_str(), "127.0.0.1");
    cr_assert_eq(addr.port(), 4242);
}

Test(address, raw_fields_are_consistent)
{
    zappy::posix::Address addr("192.168.1.10", 8080);
    const sockaddr_in &raw = addr.raw();

    cr_assert_eq(raw.sin_family, AF_INET);
    cr_assert_eq(ntohs(raw.sin_port), 8080);
    cr_assert_eq(addr.size(), static_cast<socklen_t>(sizeof(sockaddr_in)));
    cr_assert_eq(addr.raw_generic(),
        reinterpret_cast<const sockaddr *>(&raw));
}

Test(address, any_uses_inaddr_any)
{
    zappy::posix::Address addr =
        zappy::posix::Address::any(1234);

    cr_assert_eq(addr.raw().sin_addr.s_addr, htonl(INADDR_ANY));
    cr_assert_eq(addr.port(), 1234);
    cr_assert_str_eq(addr.ip().c_str(), "0.0.0.0");
}

Test(address, malformed_ip_throws)
{
    cr_assert_throw(zappy::posix::Address("not.an.ip", 1),
        std::invalid_argument);
    cr_assert_throw(zappy::posix::Address("999.0.0.1", 1),
        std::invalid_argument);
}

Test(address, is_copyable)
{
    zappy::posix::Address original("10.0.0.5", 9000);
    zappy::posix::Address copy = original;

    cr_assert_str_eq(copy.ip().c_str(), "10.0.0.5");
    cr_assert_eq(copy.port(), 9000);
}
