/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Implementation of the poll() event loop
*/

#include "net/PollLoop.hpp"
#include "net/NetworkError.hpp"

#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

/**
 * @brief Create a non-blocking eventfd used to wake the loop.
 *
 * @return zappy::posix::FileDescriptor owning the eventfd
 */
static zappy::posix::FileDescriptor makeWakeEvent()
{
    zappy::posix::FileDescriptor fd(
        ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC));

    if (!fd.is_valid())
        throw zappy::net::NetworkError(
            std::string("eventfd: ") + std::strerror(errno));
    return fd;
}

/**
 * @brief Run a loop hook, logging any exception instead of letting it escape.
 *
 * Keeps the running server alive: a runtime error raised while a hook fires
 * is reported on stderr and the loop keeps spinning.
 *
 * @param hook the hook to invoke (no-op if empty)
 * @param label human-readable hook name used in the error log
 */
static void run_hook(const std::function<void()> &hook, const char *label)
{
    if (!hook)
        return;
    try {
        hook();
    } catch (const std::exception &error) {
        std::cerr << label << " error: " << error.what() << "\n";
    }
}

/**
 * @brief Append a descriptor to the polled set.
 *
 * @param pollfds the polled set to extend
 * @param fd file descriptor to watch
 * @param events bitmask of events to monitor (POLLIN, POLLOUT, ...)
 */
static void watch(std::vector<pollfd> &pollfds, int fd, short events)
{
    pollfd entry = {};

    entry.fd = fd;
    entry.events = events;
    pollfds.push_back(entry);
}

/**
 * @brief Build the loop and register the SIGINT signalfd and wake eventfd.
 */
zappy::net::PollLoop::PollLoop()
    : _stop_requested(false), _wake_event(makeWakeEvent()), _timeout_ms(-1)
{
    watch(_pollfds, _signals.read_fd(), POLLIN);
    watch(_pollfds, _wake_event.get(), POLLIN);
}

/**
 * @brief Watch @p fd for @p events and dispatch to @p callback when ready.
 *
 * @param fd file descriptor to register
 * @param events bitmask of events to monitor (POLLIN, POLLOUT, ...)
 * @param callback function invoked with the actual revents when the fd is ready
 */
void zappy::net::PollLoop::register_fd(int fd, short events,
    Callback callback)
{
    watch(_pollfds, fd, events);
    _callbacks[fd] = std::move(callback);
}

/**
 * @brief Stop watching @p fd and drop its callback.
 *
 * @param fd file descriptor to unregister
 */
void zappy::net::PollLoop::unregister_fd(int fd)
{
    _callbacks.erase(fd);
    std::erase_if(_pollfds,
        [fd](const pollfd &entry) { return entry.fd == fd; });
}

/**
 * @brief Change the watched events of an already registered @p fd.
 *
 * @param fd file descriptor to update (no-op if not registered)
 * @param events new bitmask of events to monitor
 */
void zappy::net::PollLoop::modify_events(int fd, short events)
{
    for (pollfd &entry : _pollfds) {
        if (entry.fd == fd) {
            entry.events = events;
            return;
        }
    }
}

/**
 * @brief Set the poll timeout in ms (-1 blocks forever).
 *
 * @param ms timeout in milliseconds, -1 to block indefinitely
 */
void zappy::net::PollLoop::set_next_timeout(int ms) noexcept
{
    _timeout_ms = ms;
}

/**
 * @brief Set the hook run before each poll() wait.
 */
void zappy::net::PollLoop::set_pre_wait_hook(PreWaitHook hook)
{
    _pre_wait_hook = std::move(hook);
}

/**
 * @brief Set the hook run after each poll() wait.
 */
void zappy::net::PollLoop::set_post_wait_hook(PostWaitHook hook)
{
    _post_wait_hook = std::move(hook);
}

/**
 * @brief Drain the eventfd used to wake the loop.
 */
void zappy::net::PollLoop::drain_wake_event() noexcept
{
    std::uint64_t value = 0;

    while (::read(_wake_event.get(), &value, sizeof(value)) > 0)
        ;
}

/**
 * @brief Dispatch the descriptors flagged ready by the last poll.
 */
void zappy::net::PollLoop::dispatch()
{
    std::vector<std::pair<int, short>> ready;

    for (const pollfd &entry : _pollfds) {
        if (entry.revents != 0)
            ready.  _back(entry.fd, entry.revents);
    }
    for (const std::pair<int, short> &item : ready) {
        if (item.first == _signals.read_fd()) {
            _signals.consume();
            _stop_requested.store(true);
            continue;
        }
        if (item.first == _wake_event.get()) {
            drain_wake_event();
            continue;
        }

        const std::unordered_map<int, Callback>::iterator it =
            _callbacks.find(item.first);

        if (it == _callbacks.end())
            continue;
        const Callback callback = it->second;

        try {
            callback(item.second);
        } catch (const std::exception &error) {
            std::cerr << "callback error on fd " << item.first << ": "
                      << error.what() << "\n";
        }
    }
}

/**
 * @brief Run the event loop until stop() or SIGINT.
 */
void zappy::net::PollLoop::run()
{
    while (!_stop_requested.load()) {
        run_hook(_pre_wait_hook, "pre-wait hook");
        const int ready = ::poll(_pollfds.data(),
            static_cast<nfds_t>(_pollfds.size()), _timeout_ms);

        if (ready < 0) {
            if (errno == EINTR)
                continue;
            throw NetworkError(std::string("poll: ") + std::strerror(errno));
        }
        if (ready > 0)
            dispatch();
        run_hook(_post_wait_hook, "post-wait hook");
    }
}

/**
 * @brief Request the loop to stop and wake it up.
 */
void zappy::net::PollLoop::stop() noexcept
{
    const std::uint64_t one = 1;
    const ssize_t written = ::write(_wake_event.get(), &one, sizeof(one));

    _stop_requested.store(true);
    (void)written;
}
