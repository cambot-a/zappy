/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** ClientBuffer implementation
*/

#include "net/ClientBuffer.hpp"

#include <array>
#include <cerrno>
#include <cstddef>
#include <string_view>
#include <unistd.h>

static constexpr std::size_t READ_CHUNK_SIZE = 4096;
static constexpr std::size_t MAX_READ_BUF    = 1024U * 1024U;

/**
 * @brief Construct a new zappy::net::ClientBuffer object.
 * Take ownership of @p fd and zero-initialise the write offset.
 * @param fd is a file descriptor
 */
zappy::net::ClientBuffer::ClientBuffer(posix::FileDescriptor fd)
    : _fd(std::move(fd)), _write_offset(0)
{}

/**
 * @brief Return the raw descriptor without releasing ownership.
 *
 * @return int
 */
int zappy::net::ClientBuffer::fd() const noexcept
{
    return _fd.get();
}

/**
 * @brief Append @p data to the read buffer;
 * return ERROR if the 1 MB cap is exceeded.
 *
 * @param data information that is retrieve
 * @return zappy::net::ReadResult
 */
zappy::net::ReadResult
zappy::net::ClientBuffer::_append_and_check(std::string_view data)
{
    ReadResult result = ReadResult::OK;

    _read_buffer.append(data);
    if (_read_buffer.size() > MAX_READ_BUF)
        result = ReadResult::ERROR;
    return result;
}

/**
 * @brief Read all available bytes in a loop; EINTR retries,
 *        EAGAIN ends the loop normally
 * @return zappy::net::ReadResult
 */
zappy::net::ReadResult zappy::net::ClientBuffer::on_readable()
{
    std::array<char, READ_CHUNK_SIZE> chunk = {};
    ReadResult result = ReadResult::OK;
    bool running = true;

    while (running && result == ReadResult::OK) {
        const ssize_t n = ::read(_fd.get(), chunk.data(), chunk.size());
        if (n == 0)
            result = ReadResult::PEER_CLOSED;
        else if (n > 0) {
            result = _append_and_check(std::string_view(chunk.data(),
                static_cast<std::size_t>(n)));
            _extract_lines();
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            running = false;
        else if (errno != EINTR)
            result = ReadResult::ERROR;
    }
    return result;
}

/**
 * @brief Split the read buffer on '\n', strip trailing '\r',
 * push complete lines into ready_messages
 */
void zappy::net::ClientBuffer::_extract_lines()
{
    std::size_t pos = 0;
    std::size_t nl = _read_buffer.find('\n', pos);

    while (nl != std::string::npos) {
        std::string line = _read_buffer.substr(pos, nl - pos);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        _ready_messages.push(std::move(line));
        pos = nl + 1;
        nl = _read_buffer.find('\n', pos);
    }
    _read_buffer.erase(0, pos);
}

/**
 * @brief If write_buffer is empty and the outgoing queue is not,
 * pop the next message into write_buffer
 */
void zappy::net::ClientBuffer::_load_next_message()
{
    if (_write_buffer.empty() && !_outgoing_messages.empty()) {
        _write_buffer = std::move(_outgoing_messages.front());
        _outgoing_messages.pop_front();
        _write_offset = 0;
    }
}

/**
 * @brief Clear the current write_buffer and load the next outgoing message, if any.
 */
void zappy::net::ClientBuffer::_advance_write_buffer()
{
    _write_buffer.clear();
    _write_offset = 0;
    if (!_outgoing_messages.empty()) {
        _write_buffer = std::move(_outgoing_messages.front());
        _outgoing_messages.pop_front();
    }
}

/**
 * @brief Interpret the return value of ::write; update @p result and return true when the loop should stop.
 *
 * @param n Return value of the preceding ::write call.
 * @param result Out parameter
 * @return true
 * @return false
 */
bool zappy::net::ClientBuffer::_handle_write_result(
    std::ptrdiff_t n, WriteResult &result)
{
    bool done = false;

    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        result = WriteResult::OK;
        done = true;
    } else if (n < 0 && errno != EINTR) {
        result = WriteResult::ERROR;
        done = true;
    } else if (n >= 0) {
        _write_offset += static_cast<std::size_t>(n);
        if (_write_offset >= _write_buffer.size())
            _advance_write_buffer();
    }
    return done;
}

/**
 * @brief Drain the outgoing queue progressively;
 * EINTR retries, EAGAIN yields WriteResult::OK.
 *
 * @return zappy::net::WriteResult
 */
zappy::net::WriteResult zappy::net::ClientBuffer::on_writable()
{
    WriteResult result = WriteResult::ALL_SENT;
    bool done = false;

    _load_next_message();
    while (!_write_buffer.empty() && !done) {
        const ssize_t n = ::write(_fd.get(),
            _write_buffer.data() + _write_offset,
            _write_buffer.size() - _write_offset);
        done = _handle_write_result(static_cast<std::ptrdiff_t>(n), result);
    }
    return result;
}

/**
 * @brief Append @p msg to the outgoing queue; adds '\n' if absent.
 *
 * @param msg message that comes from client
 */
void zappy::net::ClientBuffer::queue_message(std::string msg)
{
    if (msg.empty() || msg.back() != '\n')
        msg += '\n';
    _outgoing_messages.push_back(std::move(msg));
}

/**
 * @brief Return true when write_buffer or outgoing_messages is non-empty.
 *
 * @return true
 * @return false
 */
bool zappy::net::ClientBuffer::has_pending_write() const noexcept
{
    return !_write_buffer.empty() || !_outgoing_messages.empty();
}

/**
 * @brief Return the next parsed line and remove it from the queue, or nullopt if none.
 *
 * @return std::optional<std::string>
 */
std::optional<std::string> zappy::net::ClientBuffer::pop_ready_message()
{
    std::optional<std::string> result = std::nullopt;

    if (!_ready_messages.empty()) {
        result = std::move(_ready_messages.front());
        _ready_messages.pop();
    }
    return result;
}

/**
 * @brief Return true when at least one complete line is ready for the dispatcher.
 *
 * @return true
 * @return false
 */
bool zappy::net::ClientBuffer::has_ready_messages() const noexcept
{
    return !_ready_messages.empty();
}
