/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** poll()-based event loop with per-fd callbacks
*/

#ifndef NET_POLLLOOP_HPP_
    #define NET_POLLLOOP_HPP_

    #include <poll.h>

    #include <atomic>
    #include <functional>
    #include <unordered_map>
    #include <vector>

    #include "net/SignalHandler.hpp"
    #include "posix/FileDescriptor.hpp"

namespace zappy::net {

/**
 * @brief Single-threaded poll() event loop dispatching to per-fd callbacks.
 */
class PollLoop {
public:
    /**
     * @brief Callback invoked with the ready revents of its descriptor.
     */
    typedef std::function<void(short revents)> Callback;

    /**
     * @brief Hook invoked once per loop iteration, right before poll() waits.
     */
    typedef std::function<void()> PreWaitHook;

    /**
     * @brief Hook invoked once per loop iteration, right after poll() returns.
     */
    typedef std::function<void()> PostWaitHook;

    /**
     * @brief Build the loop and register the SIGINT signalfd and wake eventfd.
     */
    PollLoop();

    PollLoop(const PollLoop &) = delete;
    PollLoop &operator=(const PollLoop &) = delete;

    /**
     * @brief Watch @p fd for @p events and dispatch to @p callback when ready.
     *
     * @param fd file descriptor to register
     * @param events bitmask of events to monitor (POLLIN, POLLOUT, ...)
     * @param callback function invoked with the actual revents when the fd is ready
     */
    void register_fd(int fd, short events, Callback callback);

    /**
     * @brief Stop watching @p fd and drop its callback.
     *
     * @param fd file descriptor to unregister
     */
    void unregister_fd(int fd);

    /**
     * @brief Change the watched events of an already registered @p fd.
     *
     * @param fd file descriptor to update (no-op if not registered)
     * @param events new bitmask of events to monitor
     */
    void modify_events(int fd, short events);

    /**
     * @brief Set the poll timeout in ms (-1 blocks forever); used later by the Scheduler.
     *
     * @param ms timeout in milliseconds, -1 to block indefinitely
     */
    void set_next_timeout(int ms) noexcept;

    /**
     * @brief Set the hook run before each poll() wait (e.g. to set the timeout).
     *
     * @param hook function called at the top of every loop iteration
     */
    void set_pre_wait_hook(PreWaitHook hook);

    /**
     * @brief Set the hook run after each poll() wait (e.g. to fire timers).
     *
     * @param hook function called at the bottom of every loop iteration
     */
    void set_post_wait_hook(PostWaitHook hook);

    /**
     * @brief Run the event loop until stop() or SIGINT.
     */
    void run();

    /**
     * @brief Request the loop to stop and wake it up; thread/signal safe.
     */
    void stop() noexcept;

private:
    /**
     * @brief Dispatch the descriptors flagged ready by the last poll.
     */
    void dispatch();

    /**
     * @brief Drain the eventfd used to wake the loop.
     */
    void drain_wake_event() noexcept;

    std::vector<pollfd> _pollfds;
    std::unordered_map<int, Callback> _callbacks;
    std::atomic<bool> _stop_requested;
    SignalHandler _signals;
    posix::FileDescriptor _wake_event;
    int _timeout_ms;
    PreWaitHook _pre_wait_hook;
    PostWaitHook _post_wait_hook;
};

} // namespace zappy::net

#endif /* !NET_POLLLOOP_HPP_ */
