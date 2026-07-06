/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Implementation of the signalfd-based SIGINT handler
*/

#include "net/SignalHandler.hpp"
#include "net/NetworkError.hpp"

#include <sys/signalfd.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>

/**
 * @brief Block SIGINT process-wide and return its non-blocking signalfd.
 *
 * @return zappy::posix::FileDescriptor owning the signalfd
 */
static zappy::posix::FileDescriptor makeSignalFd()
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    if (::sigprocmask(SIG_BLOCK, &mask, nullptr) < 0)
        throw zappy::net::NetworkError(
            std::string("sigprocmask: ") + std::strerror(errno));

    zappy::posix::FileDescriptor fd(
        ::signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC));

    if (!fd.is_valid())
        throw zappy::net::NetworkError(
            std::string("signalfd: ") + std::strerror(errno));
    return fd;
}

/**
 * @brief Block SIGINT and open its signalfd.
 */
zappy::net::SignalHandler::SignalHandler()
    : _signal_fd(makeSignalFd())
{
}

/**
 * @brief The signalfd to register in the poll loop.
 *
 * @return int the signalfd file descriptor
 */
int zappy::net::SignalHandler::read_fd() const noexcept
{
    return _signal_fd.get();
}

/**
 * @brief Drain pending signal notifications.
 */
void zappy::net::SignalHandler::consume() const noexcept
{
    signalfd_siginfo info;

    while (::read(_signal_fd.get(), &info, sizeof(info)) > 0)
        ;
}
