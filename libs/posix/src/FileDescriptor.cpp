/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Implementation of the POSIX file descriptor RAII owner
*/

#include "posix/FileDescriptor.hpp"

#include <unistd.h>
#include <utility>

static constexpr int INVALID_FD = -1;

/**
 * @brief Build an invalid descriptor.
 */
zappy::posix::FileDescriptor::FileDescriptor() noexcept
    : _fd(INVALID_FD)
{
}

/**
 * @brief Take ownership of @p fd.
 *
 * @param fd raw file descriptor to wrap (INVALID_FD for an empty descriptor)
 */
zappy::posix::FileDescriptor::FileDescriptor(int fd) noexcept : _fd(fd)
{
}

/**
 * @brief Close the owned descriptor.
 */
zappy::posix::FileDescriptor::~FileDescriptor()
{
    close();
}

/**
 * @brief Steal the descriptor from @p other.
 *
 * @param other source descriptor, left in an invalid state
 */
zappy::posix::FileDescriptor::FileDescriptor(
    FileDescriptor &&other) noexcept
    : _fd(other.release())
{
}

/**
 * @brief Close the current descriptor, then steal @p other's.
 *
 * @param other source descriptor, left in an invalid state
 * @return FileDescriptor& reference to this
 */
zappy::posix::FileDescriptor &
zappy::posix::FileDescriptor::operator=(FileDescriptor &&other) noexcept
{
    if (this != &other) {
        close();
        _fd = other.release();
    }
    return *this;
}

/**
 * @brief Return the raw descriptor without releasing ownership.
 *
 * @return int the raw file descriptor, or INVALID_FD if empty
 */
int zappy::posix::FileDescriptor::get() const noexcept
{
    return _fd;
}

/**
 * @brief Release ownership and return the raw descriptor.
 *
 * @return int the raw file descriptor; caller is now responsible for closing it
 */
int zappy::posix::FileDescriptor::release() noexcept
{
    return std::exchange(_fd, INVALID_FD);
}

/**
 * @brief Close the descriptor if open; idempotent.
 */
void zappy::posix::FileDescriptor::close() noexcept
{
    if (_fd != INVALID_FD) {
        ::close(_fd);
        _fd = INVALID_FD;
    }
}

/**
 * @brief Tell whether the descriptor is valid.
 *
 * @return true if the descriptor is open
 * @return false if the descriptor is INVALID_FD
 */
bool zappy::posix::FileDescriptor::is_valid() const noexcept
{
    return _fd != INVALID_FD;
}

/**
 * @brief Same as is_valid().
 *
 * @return true if the descriptor is open
 * @return false if the descriptor is INVALID_FD
 */
zappy::posix::FileDescriptor::operator bool() const noexcept
{
    return is_valid();
}
