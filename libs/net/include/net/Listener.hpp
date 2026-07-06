/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** TCP listening socket wrapped in RAII, ready for poll
*/

#ifndef NET_LISTENER_HPP_
    #define NET_LISTENER_HPP_

    #include <cstdint>
    #include <optional>

    #include "posix/Address.hpp"
    #include "posix/FileDescriptor.hpp"

namespace zappy::net {

/**
 * @brief Move-only non-blocking TCP listener bound to INADDR_ANY.
 */
class Listener {
public:
    /**
     * @brief Create, bind and listen on @p port.
     *
     * @param port server port (0 lets the OS pick a free port)
     * @param backlog kernel queue size for pending connections
     * @throws NetworkError on any underlying POSIX failure
     */
    explicit Listener(std::uint16_t port, int backlog = 128);

    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;
    Listener(Listener &&) noexcept = default;
    Listener &operator=(Listener &&) noexcept = default;

    /**
     * @brief The raw listening descriptor for poll registration.
     *
     * @return int the listening socket file descriptor
     */
    [[nodiscard]] int fd() const noexcept;

    /**
     * @brief The port actually bound (resolved when constructed with port 0).
     *
     * @return std::uint16_t the bound port
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

    /**
     * @brief Accept one pending connection without blocking.
     *
     * @return std::optional<posix::FileDescriptor> the accepted client socket,
     *         or std::nullopt on EAGAIN/EWOULDBLOCK
     * @throws NetworkError on any other underlying POSIX failure
     */
    [[nodiscard]] std::optional<posix::FileDescriptor> accept_connection() const;

    /**
     * @brief Resolve the peer address of an already-connected client socket.
     *
     * @param client an accepted client socket
     * @return posix::Address the remote address of the peer
     * @throws NetworkError if getpeername fails
     */
    [[nodiscard]] static posix::Address peer_address(
        const posix::FileDescriptor &client);

private:
    posix::FileDescriptor _socket;
    std::uint16_t _port;
};

} // namespace zappy::net

#endif /* !NET_LISTENER_HPP_ */
