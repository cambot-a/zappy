/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Top-level server orchestrator
*/

#ifndef SERVER_SERVER_HPP_
    #define SERVER_SERVER_HPP_

    #include "server/ai/AiDispatcher.hpp"
    #include "server/gui/GuiDispatcher.hpp"
    #include "server/gui/GuiNotifier.hpp"
    #include "server/cli/ServerConfig.hpp"
    #include "server/config/FeatureFlags.hpp"
    #include "server/event/EventBus.hpp"
    #include "server/client/ClientRegistry.hpp"
    #include "server/game/FoodScheduler.hpp"
    #include "server/game/RefillScheduler.hpp"
    #include "server/game/ResourceSpawner.hpp"
    #include "server/game/World.hpp"
    #include "server/handshake/HandshakeHandler.hpp"
    #include "server/scheduler/Clock.hpp"
    #include "server/scheduler/Scheduler.hpp"
    #include "net/Listener.hpp"
    #include "net/PollLoop.hpp"

namespace zappy::server {

/**
 * @brief Top-level orchestrator owning the listener, poll loop and client registry.
 */
class Server {
public:
    /**
     * @brief Build the server from a validated configuration.
     *
     * @param config the parsed CLI configuration
     */
    explicit Server(const cli::ServerConfig &config);

    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    /**
     * @brief Run the event loop until SIGINT or stop().
     */
    void run();

private:
    /**
     * @brief Drain all pending accept()s on the listening socket.
     */
    void acceptPendingClients();

    /**
     * @brief Dispatch POLLIN / POLLOUT events for one client fd.
     *
     * @param clientFd the file descriptor that got events
     * @param revents bitmask returned by poll for this fd
     */
    void handleClientEvents(int clientFd, short revents);

    /**
     * @brief Process a POLLIN event; return true if the client was dropped.
     *
     * @param clientFd the readable file descriptor
     * @return true if the client was dropped
     */
    bool handlePollin(int clientFd);

    /**
     * @brief Process a POLLOUT event; return true if the client was dropped.
     *
     * @param clientFd the writable file descriptor
     * @return true if the client was dropped
     */
    bool handlePollout(int clientFd);

    /**
     * @brief Unregister @p clientFd from the poll loop and remove it from the registry.
     *
     * @param clientFd file descriptor to drop
     */
    void dropClient(int clientFd);

    /**
     * @brief Adjust the poll mask to reflect whether POLLOUT is needed.
     *
     * @param clientFd file descriptor to retune
     */
    void retunePollMask(int clientFd);

    /**
     * @brief Route all ready messages from @p clientFd to the appropriate handler.
     *
     * @param clientFd file descriptor whose buffer has ready messages
     * @return true if the client was dropped during dispatch
     */
    bool dispatchReadyMessages(int clientFd);

    /**
     * @brief Handle a player reaching zero food: notify, kill and drop.
     *
     * @param playerId starving player id reported by the food scheduler
     */
    void onPlayerStarved(int playerId);

    /**
     * @brief Disconnect the AI client of a player killed by an admin command.
     *
     * The world already marked the player DEAD; this notifies and drops its
     * AI connection ("dead\n" then deferred drop once the buffer flushes).
     *
     * @param playerId id of the player killed by the GUI admin
     */
    void onPlayerKilledByAdmin(int playerId);

    /**
     * @brief Deliver an AI command response to its client, if still connected.
     *
     * @param playerId player the response is addressed to
     * @param response response line (no trailing newline)
     */
    void onAiResponse(int playerId, std::string response);

    /**
     * @brief Deliver a GUI command response to its client, if still connected.
     *
     * @param fd client file descriptor
     * @param response response line (no trailing newline)
     */
    void onGuiResponse(int fd, std::string response);

    /**
     * @brief Update the server time frequency dynamically and scale active timers.
     *
     * @param newFrequency new reciprocal of the time unit
     */
    void updateFrequency(int newFrequency);

    /**
     * @brief Read lines from standard input and broadcast them to all GUI clients.
     */
    void handleStdin();

    cli::ServerConfig _config;
    config::FeatureFlags _featureFlags;
    net::Listener _listener;
    net::PollLoop _poll;
    game::World _world;
    game::ResourceSpawner _resourceSpawner;
    client::ClientRegistry _clients;
    scheduler::SteadyClock _clock;
    scheduler::Scheduler _scheduler{_clock};
    int _frequency;
    game::RefillScheduler _refillScheduler;
    game::FoodScheduler _foodScheduler;
    ai::AiDispatcher _aiDispatcher;
    gui::GuiNotifier _guiNotifier;
    event::EventBus _eventBus;
    gui::GuiDispatcher _guiDispatcher;
    handshake::HandshakeHandler _handshake;
};

} // namespace zappy::server

#endif /* !SERVER_SERVER_HPP_ */
