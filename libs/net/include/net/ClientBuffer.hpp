/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Per-fd buffering layer between PollLoop and the dispatcher
*/

#ifndef NET_CLIENTBUFFER_HPP_
    #define NET_CLIENTBUFFER_HPP_

    #include <cstddef>
    #include <deque>
    #include <optional>
    #include <queue>
    #include <string>
    #include <string_view>

    #include "net/NetworkError.hpp"
    #include "posix/FileDescriptor.hpp"

namespace zappy::net {

/**
 * @brief Outcome of a single on_readable() pass.
 */
enum class ReadResult  { OK, PEER_CLOSED, ERROR };

/**
 * @brief Outcome of a single on_writable() pass.
 */
enum class WriteResult { OK, ERROR, ALL_SENT };

/**
 * @brief Thrown on programmer errors (e.g. calling on_readable after PEER_CLOSED).
 */
class BufferError : public NetworkError {
public:
    /**
     * @brief Build an error with a reason.
     *
     * @param reason description of the misuse
     */
    explicit BufferError(const std::string &reason) : NetworkError(reason) {}
};

/**
 * @brief Move-only per-client I/O buffer: accumulates reads, drains a write queue.
 */
class ClientBuffer {
public:
    /**
     * @brief Take ownership of @p fd; @p fd must already be O_NONBLOCK.
     *
     * @param fd owning non-blocking file descriptor
     */
    explicit ClientBuffer(posix::FileDescriptor fd);

    ClientBuffer(const ClientBuffer &) = delete;
    ClientBuffer &operator=(const ClientBuffer &) = delete;
    ClientBuffer(ClientBuffer &&) noexcept = default;
    ClientBuffer &operator=(ClientBuffer &&) noexcept = default;

    /**
     * @brief Raw descriptor for poll registration.
     *
     * @return int the file descriptor
     */
    [[nodiscard]] int fd() const noexcept;

    /**
     * @brief Read available bytes; extract newline-delimited lines into ready_messages.
     *
     * @return ReadResult OK on success, PEER_CLOSED when read returns 0, ERROR on I/O failure
     */
    ReadResult on_readable();

    /**
     * @brief Drain the outgoing queue progressively; call when POLLOUT fires.
     *
     * @return WriteResult OK if EAGAIN was hit, ERROR on I/O failure, ALL_SENT when queue is empty
     */
    WriteResult on_writable();

    /**
     * @brief Enqueue @p msg for sending; appends '\n' if absent.
     *
     * @param msg message to send (taken by value, moved into the queue)
     */
    void queue_message(std::string msg);

    /**
     * @brief Whether POLLOUT should be registered.
     *
     * @return true if bytes are pending to be written
     * @return false if the outgoing queue and the current write buffer are empty
     */
    [[nodiscard]] bool has_pending_write() const noexcept;

    /**
     * @brief Pop the next fully-received line, if any.
     *
     * @return std::optional<std::string> the next line, or std::nullopt if none ready
     */
    [[nodiscard]] std::optional<std::string> pop_ready_message();

    /**
     * @brief Whether at least one parsed line is ready for the dispatcher.
     *
     * @return true if pop_ready_message() would return a value
     * @return false otherwise
     */
    [[nodiscard]] bool has_ready_messages() const noexcept;

private:
    /**
     * @brief Append @p data to the read buffer and enforce the size cap.
     *
     * @param data chunk freshly read from the fd
     * @return ReadResult OK if within the cap, ERROR if the cap was exceeded
     */
    ReadResult _append_and_check(std::string_view data);

    /**
     * @brief Extract every complete '\n'-terminated line from the read buffer into ready_messages.
     */
    void _extract_lines();

    /**
     * @brief Move the next outgoing message into the write buffer (no-op if queue is empty).
     */
    void _load_next_message();

    /**
     * @brief Interpret the return value of ::write; update @p result and return true when the loop should stop.
     *
     * @param n return value of the preceding ::write call
     * @param result out parameter set to OK on EAGAIN, ERROR on real failure, untouched otherwise
     * @return true if the write loop must stop
     * @return false if the loop should continue
     */
    bool _handle_write_result(std::ptrdiff_t n, WriteResult &result);

    /**
     * @brief Advance past the current write buffer once fully sent and load the next message.
     */
    void _advance_write_buffer();

    posix::FileDescriptor _fd;
    std::string _read_buffer;
    std::deque<std::string> _outgoing_messages;
    std::string _write_buffer;
    std::size_t _write_offset;
    std::queue<std::string> _ready_messages;
};

} // namespace zappy::net

#endif /* !NET_CLIENTBUFFER_HPP_ */
