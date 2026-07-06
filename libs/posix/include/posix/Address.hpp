/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Value wrapper over sockaddr_in usable with POSIX socket APIs
*/

#ifndef POSIX_ADDRESS_HPP_
    #define POSIX_ADDRESS_HPP_

    #include <netinet/in.h>
    #include <sys/socket.h>

    #include <cstdint>
    #include <string>

namespace zappy::posix {

/**
 * @brief Copyable value wrapper over an IPv4 sockaddr_in.
 */
class Address {
public:
    /**
     * @brief Build from a dotted IPv4 string; validates the IP.
     *
     * @param ip dotted IPv4 address (e.g. "127.0.0.1")
     * @param port port in host byte order
     * @throws std::invalid_argument if @p ip is not a valid IPv4 address
     */
    Address(const std::string &ip, std::uint16_t port);

    /**
     * @brief Build from an existing sockaddr_in.
     *
     * @param raw existing sockaddr_in to wrap
     */
    explicit Address(const sockaddr_in &raw) noexcept;

    /**
     * @brief Build a wildcard address (INADDR_ANY) on @p port.
     *
     * @param port port in host byte order
     * @return Address bound to INADDR_ANY:port
     */
    [[nodiscard]] static Address any(std::uint16_t port) noexcept;

    /**
     * @brief Return the dotted IPv4 string.
     *
     * @return std::string the IPv4 address in dotted decimal notation
     */
    [[nodiscard]] std::string ip() const;

    /**
     * @brief Return the port in host byte order.
     *
     * @return std::uint16_t the port
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

    /**
     * @brief Return the underlying sockaddr_in.
     *
     * @return const sockaddr_in& reference to the wrapped sockaddr_in
     */
    [[nodiscard]] const sockaddr_in &raw() const noexcept;

    /**
     * @brief Return the address as a generic sockaddr for bind/connect/accept.
     *
     * @return const sockaddr* pointer to the wrapped address as sockaddr
     */
    [[nodiscard]] const sockaddr *raw_generic() const noexcept;

    /**
     * @brief Return the size of the underlying sockaddr_in.
     *
     * @return socklen_t size of sockaddr_in in bytes
     */
    [[nodiscard]] socklen_t size() const noexcept;

private:
    sockaddr_in _addr;
};

}

#endif /* !POSIX_ADDRESS_HPP_ */
