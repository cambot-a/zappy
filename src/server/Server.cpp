/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Server implementation
*/

#include "server/Server.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server/game/EggSpawner.hpp"
#include "server/game/BiomeMapGenerator.hpp"
#include "server/config/FeatureFlag.hpp"

namespace {

/**
 * @brief Resolve the machine's primary outbound IPv4 address.
 *
 * Opens a UDP socket and "connects" it to a public address: no packet is
 * sent, but the kernel picks the source interface, which getsockname() then
 * reveals. Falls back to the loopback address when offline or on any error.
 *
 * @return std::string the local IPv4 in dotted form, or "127.0.0.1"
 */
std::string resolveLocalIp()
{
    const int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    std::string ip = "127.0.0.1";
    sockaddr_in probe = {};
    sockaddr_in local = {};
    socklen_t len = sizeof(local);
    char buf[INET_ADDRSTRLEN] = {};

    if (sock < 0)
        return ip;
    probe.sin_family = AF_INET;
    probe.sin_port = htons(53);
    ::inet_pton(AF_INET, "8.8.8.8", &probe.sin_addr);
    if (::connect(sock, reinterpret_cast<sockaddr *>(&probe), sizeof(probe)) == 0
        && ::getsockname(sock, reinterpret_cast<sockaddr *>(&local), &len) == 0
        && ::inet_ntop(AF_INET, &local.sin_addr, buf, sizeof(buf)) != nullptr)
        ip = buf;
    ::close(sock);
    return ip;
}

} // namespace

/**
 * @brief Initialise all members from the validated configuration.
 *
 * Member order matches the declaration order in Server.hpp:
 * _config   _listener   _poll   _world   _resourceSpawner   _clients  
 * _clock   _scheduler   _refillScheduler   _foodScheduler   _aiDispatcher  
 * _handshake.
 *
 * @param config validated CLI configuration
 */
zappy::server::Server::Server(const cli::ServerConfig &config)
    : _config(config),
      _featureFlags(),
      _listener(_config.port()),
      _poll(),
      _world(_config.width(), _config.height(),
          _config.teamNames(), _config.clientsPerTeam()),
      _resourceSpawner(),
      _clients(),
      _scheduler(_clock),
      _frequency(_config.frequency()),
      _refillScheduler(_world, _resourceSpawner, _scheduler, _frequency),
      _foodScheduler(_world, _scheduler, _frequency,
          [this](int playerId) { onPlayerStarved(playerId); }),
      _aiDispatcher(_world, _scheduler, _frequency,
          [this](int id, std::string r) { onAiResponse(id, std::move(r)); }),
      _guiNotifier(_clients, _world, [this](int fd) { retunePollMask(fd); }),
      _eventBus(_world, _guiNotifier, _scheduler, _featureFlags),
      _guiDispatcher(_world,
          gui::GuiAdminDeps{_clients, _featureFlags, _config.adminPassword(),
              [this](int playerId) { onPlayerKilledByAdmin(playerId); },
              _eventBus,
              [this](int id) { return _aiDispatcher.adminStartIncantation(id); },
              [this](int id) { return _aiDispatcher.adminStopIncantation(id); }},
          [this](int fd, std::string r) { onGuiResponse(fd, std::move(r)); },
          [this] { return _frequency; },
          [this](int freq) { updateFrequency(freq); }),
      _handshake(_config, _world, _foodScheduler)
{
    for (const auto flag : _config.initialEnabledFlags())
        _featureFlags.enable(flag);
    std::cout << "Feature flags: "
              << (_config.initialEnabledFlags().empty()
                  ? "all disabled (strict mode)" : "some enabled via CLI")
              << "\n";
    if (_featureFlags.isEnabled(config::FeatureFlag::BIOMES)) {
        game::BiomeMapGenerator biomeGen;
        biomeGen.generate(_world);
    }
    _resourceSpawner.spawnInitial(_world);
    game::EggSpawner eggSpawner;
    eggSpawner.spawnInitial(_world);
    _refillScheduler.start();
    _world.addObserver(_guiNotifier);
    _eventBus.start();
}

/**
 * @brief Register the accept callback and enter the poll loop.
 */
void zappy::server::Server::run()
{
    std::cout << "zappy_server listening on "
              << resolveLocalIp() << ":" << _listener.port() << "\n";
    _poll.register_fd(_listener.fd(), POLLIN, [this](short) {
        acceptPendingClients();
    });
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    _poll.register_fd(0, POLLIN, [this](short) {
        handleStdin();
    });
    _poll.set_pre_wait_hook([this] {
        _poll.set_next_timeout(_scheduler.nextTimeoutMs());
    });
    _poll.set_post_wait_hook([this] { _scheduler.tick(); });
    _poll.run();
    std::cout << "zappy_server shutting down\n";
}

/**
 * @brief Accept all pending connections, send WELCOME, and register each in the poll loop.
 */
void zappy::server::Server::acceptPendingClients()
{
    std::optional<posix::FileDescriptor> conn =
        _listener.accept_connection();
    while (conn) {
        const int cfd = conn->get();
        const posix::Address peer =
            net::Listener::peer_address(*conn);
        client::Client &c = _clients.add(std::move(*conn));
        c.buffer().queue_message("WELCOME");
        std::cout << "client connected from "
                  << peer.ip() << ":" << peer.port() << "\n";
        _poll.register_fd(cfd, POLLIN, [this, cfd](short revents) {
            handleClientEvents(cfd, revents);
        });
        retunePollMask(cfd);
        conn = _listener.accept_connection();
    }
}

/**
 * @brief Dispatch events for @p clientFd; skip if already removed from registry.
 *
 * @param clientFd file descriptor that received events
 * @param revents bitmask of events returned by poll
 */
void zappy::server::Server::handleClientEvents(
    int clientFd, short revents)
{
    bool dropped = !_clients.contains(clientFd);
    if (!dropped && (revents & POLLIN))
        dropped = handlePollin(clientFd);
    if (!dropped && (revents & POLLOUT))
        dropped = handlePollout(clientFd);
    if (!dropped)
        retunePollMask(clientFd);
    if (!dropped && _clients.get(clientFd).isMarkedForDrop() &&
        !_clients.get(clientFd).buffer().has_pending_write())
        dropClient(clientFd);
}

/**
 * @brief Read available data, then dispatch ready messages; drop client on error or EOF.
 *
 * @param clientFd readable file descriptor
 * @return true if the client was dropped
 */
bool zappy::server::Server::handlePollin(int clientFd)
{
    const auto result =
        _clients.get(clientFd).buffer().on_readable();
    bool dropped = result != net::ReadResult::OK;
    if (dropped) {
        std::cerr << "client " << clientFd << " disconnected\n";
        dropClient(clientFd);
    }
    if (!dropped)
        dropped = dispatchReadyMessages(clientFd);
    return dropped;
}

/**
 * @brief Drain the write queue; drop client on write error.
 *
 * @param clientFd writable file descriptor
 * @return true if the client was dropped
 */
bool zappy::server::Server::handlePollout(int clientFd)
{
    const auto result =
        _clients.get(clientFd).buffer().on_writable();
    bool dropped = result == net::WriteResult::ERROR;
    if (dropped) {
        std::cerr << "client " << clientFd << " write error\n";
        dropClient(clientFd);
    }
    return dropped;
}

/**
 * @brief Kill the player (if AI and still alive), unregister and erase the client.
 *
 * Idempotent: a no-op when @p clientFd is already gone. Stops food consumption
 * and kills the player only when it is still alive.
 *
 * @param clientFd file descriptor to drop
 */
void zappy::server::Server::dropClient(int clientFd)
{
    if (!_clients.contains(clientFd))
        return;
    client::Client &client = _clients.get(clientFd);
    if (client.state() == client::ClientState::AI) {
        const int playerId = client.aiData().playerId;
        _aiDispatcher.stopPlayer(playerId);
        _foodScheduler.stopConsumption(playerId);
        if (_world.hasPlayer(playerId) &&
            _world.player(playerId).state() != game::PlayerState::DEAD)
            _world.killPlayer(playerId);
    }
    _poll.unregister_fd(clientFd);
    _clients.remove(clientFd);
}

/**
 * @brief React to starvation: send "dead", flag for drop, kill and arm POLLOUT.
 *
 * @param playerId starving player id reported by the food scheduler
 */
void zappy::server::Server::onPlayerStarved(int playerId)
{
    const std::optional<int> fd = _clients.findFdByPlayerId(playerId);
    if (!fd)
        return;
    _aiDispatcher.stopPlayer(playerId);
    client::Client &client = _clients.get(*fd);
    client.buffer().queue_message("dead");
    client.markForDrop();
    if (_world.hasPlayer(playerId))
        _world.killPlayer(playerId);
    retunePollMask(*fd);
}

/**
 * @brief Disconnect the AI client of a player killed by an admin command.
 *
 * @param playerId id of the player killed by the GUI admin
 */
void zappy::server::Server::onPlayerKilledByAdmin(int playerId)
{
    const std::optional<int> fd = _clients.findFdByPlayerId(playerId);
    if (!fd)
        return;
    _aiDispatcher.stopPlayer(playerId);
    client::Client &client = _clients.get(*fd);
    client.buffer().queue_message("dead");
    client.markForDrop();
    retunePollMask(*fd);
}

/**
 * @brief Deliver @p response to the AI client owning @p playerId, if present.
 *
 * @param playerId player the response is addressed to
 * @param response response line (no trailing newline)
 */
void zappy::server::Server::onAiResponse(int playerId, std::string response)
{
    const std::optional<int> fd = _clients.findFdByPlayerId(playerId);
    if (!fd)
        return;
    _clients.get(*fd).buffer().queue_message(std::move(response));
    retunePollMask(*fd);
}

/**
 * @brief Deliver a GUI command response to its client, if still connected.
 *
 * @param fd client file descriptor
 * @param response response line (no trailing newline)
 */
void zappy::server::Server::onGuiResponse(int fd, std::string response)
{
    if (_clients.contains(fd)) {
        _clients.get(fd).buffer().queue_message(std::move(response));
        retunePollMask(fd);
    }
}

/**
 * @brief Recompute POLLOUT need and update the poll mask for @p clientFd.
 *
 * @param clientFd file descriptor to retune
 */
void zappy::server::Server::retunePollMask(int clientFd)
{
    const bool wantWrite =
        _clients.get(clientFd).buffer().has_pending_write();
    const short events =
        static_cast<short>(POLLIN | (wantWrite ? POLLOUT : 0));
    _poll.modify_events(clientFd, events);
}

/**
 * @brief Route each ready message to the appropriate handler based on client state.
 *
 * For HANDSHAKE: delegates to HandshakeHandler; drops the client on DROP.
 * For other states: messages are silently consumed (real dispatchers are future work).
 *
 * @param clientFd file descriptor with ready messages
 * @return true if the client was dropped during dispatch
 */
bool zappy::server::Server::dispatchReadyMessages(int clientFd)
{
    client::Client &c = _clients.get(clientFd);
    bool dropped = false;
    while (!dropped && c.buffer().has_ready_messages()) {
        const auto msg = c.buffer().pop_ready_message();
        if (c.state() == client::ClientState::HANDSHAKE) {
            const auto res = _handshake.handle(c, *msg);
            dropped = (res == handshake::HandshakeResult::DROP);
            if (dropped) {
                dropClient(clientFd);
            } else if (c.state() == client::ClientState::GUI) {
                _guiDispatcher.sendInitialSync(clientFd, _frequency);
            }
        } else if (c.state() == client::ClientState::AI)
            _aiDispatcher.dispatch(c.aiData().playerId, *msg);
        else if (c.state() == client::ClientState::GUI
            || c.state() == client::ClientState::GUI_ADMIN)
            _guiDispatcher.dispatch(clientFd, *msg);
    }
    return dropped;
}

/**
 * @brief Update the server time frequency dynamically and scale active timers.
 *
 * @param newFrequency new reciprocal of the time unit
 */
void zappy::server::Server::updateFrequency(int newFrequency)
{
    if (newFrequency <= 0)
        return;
    const double factor = static_cast<double>(_frequency) / newFrequency;
    _frequency = newFrequency;
    _refillScheduler.setFrequency(newFrequency);
    _foodScheduler.setFrequency(newFrequency);
    _aiDispatcher.setFrequency(newFrequency);
    _scheduler.rescaleDelays(factor);
    _guiNotifier.broadcast("sst " + std::to_string(newFrequency));
}

/**
 * @brief Read lines from standard input and broadcast them to all GUI clients.
 */
void zappy::server::Server::handleStdin()
{
    char buf[1024];
    const ssize_t n = read(0, buf, sizeof(buf) - 1);

    if (n <= 0) {
        _poll.unregister_fd(0);
        return;
    }
    buf[n] = '\0';
    std::string line(buf);
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();
    if (!line.empty())
        _guiNotifier.broadcastMessage(line);
}
