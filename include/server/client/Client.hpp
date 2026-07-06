/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Per-fd aggregate: buffer, state machine, and per-state payload
*/

#ifndef CLIENT_CLIENT_HPP_
    #define CLIENT_CLIENT_HPP_

    #include "server/client/ClientPayloads.hpp"
    #include "server/client/ClientState.hpp"
    #include "server/client/ClientStateError.hpp"
    #include "net/ClientBuffer.hpp"
    #include "posix/FileDescriptor.hpp"

namespace zappy::server::client {

/**
 * @brief Move-only per-client aggregate: owns the I/O buffer, tracks the state,
 *        and holds the per-state payload.
 */
class Client {
public:
    /**
     * @brief Take ownership of @p fd; initial state is HANDSHAKE with monostate payload.
     *
     * @param fd non-blocking file descriptor (already accepted, O_NONBLOCK set)
     */
    explicit Client(posix::FileDescriptor fd);

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&) noexcept = default;
    Client &operator=(Client &&) noexcept = default;

    /**
     * @brief Raw descriptor forwarded from the internal buffer.
     *
     * @return int the file descriptor
     */
    [[nodiscard]] int fd() const noexcept;

    /**
     * @brief Current lifecycle state of this client.
     *
     * @return ClientState
     */
    [[nodiscard]] ClientState state() const noexcept;

    /**
     * @brief Mutable access to the I/O buffer.
     *
     * @return net::ClientBuffer&
     */
    [[nodiscard]] net::ClientBuffer &buffer() noexcept;

    /**
     * @brief Read-only access to the I/O buffer.
     *
     * @return const net::ClientBuffer&
     */
    [[nodiscard]] const net::ClientBuffer &buffer() const noexcept;

    /**
     * @brief Transition from HANDSHAKE to @p newState and initialise the matching payload.
     *
     * @param newState target state; must be AI, GUI, or GUI_ADMIN
     * @throws ClientStateError if current state is not HANDSHAKE or newState is invalid
     */
    void promote(ClientState newState);

    /**
     * @brief Promote a GUI client to GUI_ADMIN; idempotent if already admin.
     *
     * @throws ClientStateError if current state is not GUI or GUI_ADMIN
     */
    void promoteToAdmin();

    /**
     * @brief Access the AiData payload.
     *
     * @throws ClientStateError if state() != AI
     */
    [[nodiscard]] AiData &aiData();

    /** @copydoc aiData() */
    [[nodiscard]] const AiData &aiData() const;

    /**
     * @brief Access the GuiData payload.
     *
     * @throws ClientStateError if state() != GUI
     */
    [[nodiscard]] GuiData &guiData();

    /** @copydoc guiData() */
    [[nodiscard]] const GuiData &guiData() const;

    /**
     * @brief Access the GuiAdminData payload.
     *
     * @throws ClientStateError if state() != GUI_ADMIN
     */
    [[nodiscard]] GuiAdminData &guiAdminData();

    /** @copydoc guiAdminData() */
    [[nodiscard]] const GuiAdminData &guiAdminData() const;

    /**
     * @brief Flag this client to be dropped once its write buffer drains.
     */
    void markForDrop() noexcept;

    /**
     * @brief Whether this client is pending a deferred drop.
     *
     * @return bool true if markForDrop() was called
     */
    [[nodiscard]] bool isMarkedForDrop() const noexcept;

private:
    net::ClientBuffer _buffer;
    ClientState _state;
    ClientPayload _payload;
    bool _markedForDrop = false;
};

} // namespace zappy::server::client

#endif /* !CLIENT_CLIENT_HPP_ */
