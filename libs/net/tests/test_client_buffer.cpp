/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Criterion unit tests for ClientBuffer using socketpair
*/

#include <criterion/criterion.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <vector>

#include "net/ClientBuffer.hpp"
#include "posix/FileDescriptor.hpp"

namespace {

/// @brief Create an AF_UNIX socketpair; sv[0] is set O_NONBLOCK for ClientBuffer.
void make_pair(int sv[2])
{
    cr_assert_eq(::socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
    const int flags = ::fcntl(sv[0], F_GETFL);
    cr_assert_neq(flags, -1);
    cr_assert_eq(::fcntl(sv[0], F_SETFL, flags | O_NONBLOCK), 0);
}

/// @brief Set a raw fd non-blocking.
void set_nonblocking(int fd)
{
    const int flags = ::fcntl(fd, F_GETFL);
    cr_assert_neq(flags, -1);
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/// @brief Convenience: construct a ClientBuffer owning sv[0].
zappy::net::ClientBuffer make_buf(int fd)
{
    return zappy::net::ClientBuffer{
        zappy::posix::FileDescriptor{fd}};
}

} // namespace

Test(client_buffer, single_complete_line, .timeout = 2)
{
    int sv[2];
    make_pair(sv);

    const char *msg = "hello\n";
    cr_assert_eq(::write(sv[1], msg, ::strlen(msg)), (ssize_t)::strlen(msg));

    auto buf = make_buf(sv[0]);

    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::OK);
    cr_assert(buf.has_ready_messages());

    const auto line = buf.pop_ready_message();
    cr_assert(line.has_value());
    cr_assert_str_eq(line->c_str(), "hello");
    cr_assert(!buf.has_ready_messages());

    ::close(sv[1]);
}

Test(client_buffer, multiple_lines_one_read, .timeout = 2)
{
    int sv[2];
    make_pair(sv);

    const char *msg = "foo\nbar\nbaz\n";
    cr_assert_eq(::write(sv[1], msg, ::strlen(msg)), (ssize_t)::strlen(msg));

    auto buf = make_buf(sv[0]);

    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::OK);

    const auto a = buf.pop_ready_message();
    const auto b = buf.pop_ready_message();
    const auto c = buf.pop_ready_message();
    cr_assert(a.has_value() && b.has_value() && c.has_value());
    cr_assert_str_eq(a->c_str(), "foo");
    cr_assert_str_eq(b->c_str(), "bar");
    cr_assert_str_eq(c->c_str(), "baz");
    cr_assert(!buf.has_ready_messages());

    ::close(sv[1]);
}

Test(client_buffer, partial_line_across_two_reads, .timeout = 2)
{
    int sv[2];
    make_pair(sv);

    auto buf = make_buf(sv[0]);

    cr_assert_eq(::write(sv[1], "hel", 3), 3);
    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::OK);
    cr_assert(!buf.has_ready_messages());

    cr_assert_eq(::write(sv[1], "lo\n", 3), 3);
    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::OK);
    cr_assert(buf.has_ready_messages());

    const auto line = buf.pop_ready_message();
    cr_assert(line.has_value());
    cr_assert_str_eq(line->c_str(), "hello");

    ::close(sv[1]);
}

Test(client_buffer, eagain_returns_ok_no_message, .timeout = 2)
{
    int sv[2];
    make_pair(sv);

    auto buf = make_buf(sv[0]);

    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::OK);
    cr_assert(!buf.has_ready_messages());

    ::close(sv[1]);
}

Test(client_buffer, peer_closed_returns_peer_closed, .timeout = 2)
{
    int sv[2];
    make_pair(sv);
    ::close(sv[1]);

    auto buf = make_buf(sv[0]);

    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::PEER_CLOSED);
}

Test(client_buffer, crlf_stripping, .timeout = 2)
{
    int sv[2];
    make_pair(sv);

    const char *msg = "hello\r\n";
    cr_assert_eq(::write(sv[1], msg, ::strlen(msg)), (ssize_t)::strlen(msg));

    auto buf = make_buf(sv[0]);

    cr_assert_eq(buf.on_readable(), zappy::net::ReadResult::OK);

    const auto line = buf.pop_ready_message();
    cr_assert(line.has_value());
    cr_assert_str_eq(line->c_str(), "hello");

    ::close(sv[1]);
}

Test(client_buffer, multiple_messages_queued_and_sent, .timeout = 2)
{
    int sv[2];
    make_pair(sv);
    set_nonblocking(sv[1]);

    auto buf = make_buf(sv[0]);

    buf.queue_message("alpha");
    buf.queue_message("beta");
    buf.queue_message("gamma");

    cr_assert(buf.has_pending_write());
    cr_assert_eq(buf.on_writable(), zappy::net::WriteResult::ALL_SENT);
    cr_assert(!buf.has_pending_write());

    std::vector<char> received;
    char tmp[256];
    ssize_t n;
    while ((n = ::read(sv[1], tmp, sizeof(tmp))) > 0)
        received.insert(received.end(), tmp, tmp + n);

    const std::string got(received.begin(), received.end());
    cr_assert_str_eq(got.c_str(), "alpha\nbeta\ngamma\n");

    ::close(sv[1]);
}

Test(client_buffer, queue_message_appends_newline_automatically, .timeout = 2)
{
    int sv[2];
    make_pair(sv);
    set_nonblocking(sv[1]);

    auto buf = make_buf(sv[0]);

    buf.queue_message("no_newline");
    buf.on_writable();

    char tmp[64] = {};
    const ssize_t n = ::read(sv[1], tmp, sizeof(tmp) - 1);
    cr_assert_gt(n, 0);
    cr_assert_str_eq(tmp, "no_newline\n");

    ::close(sv[1]);
}

Test(client_buffer, write_eagain_then_completes_after_drain, .timeout = 5)
{
    int sv[2];
    make_pair(sv);

    {
        const std::vector<char> filler(4096, 'F');
        while (::write(sv[0], filler.data(), filler.size()) > 0) {}
    }

    auto buf = make_buf(sv[0]);
    buf.queue_message("test");
    cr_assert(buf.has_pending_write());

    cr_assert_eq(buf.on_writable(), zappy::net::WriteResult::OK);
    cr_assert(buf.has_pending_write());

    set_nonblocking(sv[1]);
    {
        char drain[65536];
        while (::read(sv[1], drain, sizeof(drain)) > 0) {}
    }

    cr_assert_eq(buf.on_writable(), zappy::net::WriteResult::ALL_SENT);
    cr_assert(!buf.has_pending_write());

    ::close(sv[1]);
}

Test(client_buffer, read_buffer_overflow_returns_error, .timeout = 10)
{
    int sv[2];
    make_pair(sv);
    set_nonblocking(sv[1]);

    auto buf = make_buf(sv[0]);

    const std::vector<char> chunk(4096, 'X');
    zappy::net::ReadResult result = zappy::net::ReadResult::OK;

    while (result == zappy::net::ReadResult::OK) {
        while (::write(sv[1], chunk.data(), chunk.size()) > 0) {}
        result = buf.on_readable();
    }

    cr_assert_eq(result, zappy::net::ReadResult::ERROR);

    ::close(sv[1]);
}
