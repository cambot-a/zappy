/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** fd-keyed registry of all connected clients
*/

#ifndef CLIENT_CLIENTREGISTRY_HPP_
    #define CLIENT_CLIENTREGISTRY_HPP_

    #include <cstddef>
    #include <optional>
    #include <unordered_map>

    #include "server/client/Client.hpp"
    #include "server/client/ClientState.hpp"
    #include "server/client/ClientStateError.hpp"
    #include "posix/FileDescriptor.hpp"

namespace zappy::server::client {

/**
 * @brief fd-keyed registry owning every connected Client.
 *
 */
class ClientRegistry {
public:
    ClientRegistry() = default;
    ClientRegistry(const ClientRegistry &) = delete;
    ClientRegistry &operator=(const ClientRegistry &) = delete;
    ClientRegistry(ClientRegistry &&) noexcept = default;
    ClientRegistry &operator=(ClientRegistry &&) noexcept = default;

    /**
     * @brief Construct a Client in HANDSHAKE state and insert it.
     *
     * @param fd non-blocking file descriptor to own
     * @return Client& reference to the newly inserted client
     */
    Client &add(posix::FileDescriptor fd);

    /**
     * @brief Remove the client for @p fd; no-op if absent.
     *
     * @param fd file descriptor to remove
     */
    void remove(int fd) noexcept;

    /**
     * @brief Whether @p fd is registered.
     *
     * @param fd file descriptor to query
     * @return true if present
     */
    [[nodiscard]] bool contains(int fd) const noexcept;

    /**
     * @brief Return the client for @p fd.
     *
     * @param fd file descriptor to look up
     * @throws ClientStateError if @p fd is not registered
     */
    [[nodiscard]] Client &get(int fd);

    /** @copydoc get(int) */
    [[nodiscard]] const Client &get(int fd) const;

    /**
     * @brief Number of registered clients.
     *
     * @return std::size_t
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Number of clients currently in @p s.
     *
     * @param s state to count
     * @return std::size_t
     */
    [[nodiscard]] std::size_t countInState(ClientState s) const noexcept;

    /**
     * @brief File descriptor of the AI client owning @p playerId.
     *
     * @param playerId game player id to match
     * @return std::optional<int> the fd, or nullopt if no AI client matches
     */
    [[nodiscard]] std::optional<int> findFdByPlayerId(int playerId) const;

    /**
     * @brief Invoke @p callback(Client&) for every registered client.
     *
     * A throwing callback propagates the exception without
     * corrupting the registry.
     *
     * @tparam F callable accepting Client&
     */
    template<typename F>
    void forEach(F &&callback)
    {
        for (auto &[key, client] : _clients)
            callback(client);
    }

    /**
     * @brief Invoke @p callback(Client&) for every client in @p state.
     *
     * @tparam F callable accepting Client&
     * @param state filter
     */
    template<typename F>
    void forEachInState(ClientState state, F &&callback)
    {
        for (auto &[key, client] : _clients) {
            if (client.state() == state)
                callback(client);
        }
    }

private:
    std::unordered_map<int, Client> _clients;
};

} // namespace zappy::server::client

#endif /* !CLIENT_CLIENTREGISTRY_HPP_ */
