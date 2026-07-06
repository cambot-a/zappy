/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** SIGINT delivery as a pollable file descriptor (signalfd), no global state
*/

#ifndef NET_SIGNALHANDLER_HPP_
    #define NET_SIGNALHANDLER_HPP_

    #include "posix/FileDescriptor.hpp"

namespace zappy::net {

/**
 * @brief Turns SIGINT into a readable descriptor via signalfd; no globals.
 */
class SignalHandler {
public:
    /**
     * @brief Block SIGINT and open its signalfd.
     */
    SignalHandler();

    SignalHandler(const SignalHandler &) = delete;
    SignalHandler &operator=(const SignalHandler &) = delete;
    SignalHandler(SignalHandler &&) noexcept = default;
    SignalHandler &operator=(SignalHandler &&) noexcept = default;

    /**
     * @brief The signalfd to register in the poll loop.
     *
     * @return int the signalfd file descriptor
     */
    [[nodiscard]] int read_fd() const noexcept;

    /**
     * @brief Drain pending signal notifications.
     */
    void consume() const noexcept;

private:
    posix::FileDescriptor _signal_fd;
};

} // namespace zappy::net

#endif /* !NET_SIGNALHANDLER_HPP_ */
