/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** ClientRegistry implementation
*/

#include "server/client/ClientRegistry.hpp"
#include <algorithm>
#include <string>

/**
 * @brief Construct a Client in HANDSHAKE state for @p fd and insert it into the registry.
 *
 * @param fd non-blocking file descriptor to own
 * @return Client& reference to the newly inserted client
 */
zappy::server::client::Client &
zappy::server::client::ClientRegistry::add(posix::FileDescriptor fd)
{
    const int key = fd.get();
    auto result = _clients.try_emplace(key, std::move(fd));
    return result.first->second;
}

/**
 * @brief Remove the client for @p fd; no-op if absent.
 *
 * @param fd file descriptor to remove
 */
void zappy::server::client::ClientRegistry::remove(int fd) noexcept
{
    _clients.erase(fd);
}

/**
 * @brief Return true when @p fd is currently registered.
 *
 * @param fd file descriptor to query
 * @return bool
 */
bool zappy::server::client::ClientRegistry::contains(int fd) const noexcept
{
    return _clients.count(fd) > 0;
}

/**
 * @brief Return a mutable reference to the client for @p fd.
 *
 * @param fd file descriptor to look up
 * @return Client&
 * @throws ClientStateError if @p fd is not registered
 */
zappy::server::client::Client &
zappy::server::client::ClientRegistry::get(int fd)
{
    auto it = _clients.find(fd);
    if (it == _clients.end())
        throw ClientStateError("ClientRegistry::get: unknown fd " + std::to_string(fd));
    return it->second;
}

/**
 * @brief Return a read-only reference to the client for @p fd.
 *
 * @param fd file descriptor to look up
 * @return const Client&
 * @throws ClientStateError if @p fd is not registered
 */
const zappy::server::client::Client &
zappy::server::client::ClientRegistry::get(int fd) const
{
    auto it = _clients.find(fd);
    if (it == _clients.end())
        throw ClientStateError("ClientRegistry::get: unknown fd " + std::to_string(fd));
    return it->second;
}

/**
 * @brief Return the number of registered clients.
 *
 * @return std::size_t
 */
std::size_t zappy::server::client::ClientRegistry::size() const noexcept
{
    return _clients.size();
}

/**
 * @brief Return the number of clients currently in state @p s.
 *
 * @param s state to count
 * @return std::size_t
 */
std::size_t
zappy::server::client::ClientRegistry::countInState(ClientState s) const noexcept
{
    std::size_t count = 0;
    for (const auto &[key, client] : _clients) {
        if (client.state() == s)
            ++count;
    }
    return count;
}

/**
 * @brief Return the fd of the AI client owning @p playerId, if any.
 *
 * @param playerId game player id to match
 * @return std::optional<int> the fd, or nullopt when no AI client matches
 */
std::optional<int>
zappy::server::client::ClientRegistry::findFdByPlayerId(int playerId) const
{
    const auto it = std::find_if(_clients.begin(), _clients.end(),
        [playerId](const auto &entry) {
            return entry.second.state() == ClientState::AI &&
                entry.second.aiData().playerId == playerId;
        });
    return it == _clients.end()
        ? std::nullopt : std::optional<int>(it->first);
}
