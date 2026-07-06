/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** GUI <-> server TCP link implementation
*/

#include "gui/net/ServerLink.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <variant>

#include "posix/FileDescriptor.hpp"

namespace zappy::gui::net {

namespace {

constexpr std::string_view WELCOME_LINE = "WELCOME";
// constexpr std::string_view GRAPHIC_CMD  = "GRAPHIC\n";
constexpr int CONNECT_TIMEOUT_MS = 2000;

[[noreturn]] void fail(const std::string &reason)
{
    throw ServerLinkError(reason);
}

[[noreturn]] void fail_errno(const std::string &reason)
{
    throw ServerLinkError(reason + ": " + std::strerror(errno));
}

void set_nonblocking(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        fail_errno("fcntl(F_GETFL)");
    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        fail_errno("fcntl(F_SETFL, O_NONBLOCK)");
}

int wait_writable(int fd, int timeout_ms)
{
    struct pollfd pfd{fd, POLLOUT, 0};
    int r = ::poll(&pfd, 1, timeout_ms);
    if (r < 0)
        fail_errno("poll(connect)");
    if (r == 0)
        fail("connect timed out");
    return pfd.revents;
}

void finalize_connect(int fd)
{
    int err = 0;
    socklen_t len = sizeof(err);
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
        fail_errno("getsockopt(SO_ERROR)");
    if (err != 0)
        fail(std::string("connect failed: ") + std::strerror(err));
}

} // namespace

ServerLink::ServerLink(const std::string &host, std::uint16_t port)
    : _buffer(nullptr), _state(HandshakeState::WAIT_WELCOME),
      _connected(false)
{
    open_socket(host, port);
    _connected = true;
}

void ServerLink::open_socket(const std::string &host, std::uint16_t port)
{
    struct addrinfo hints{};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_NUMERICSERV;

    struct addrinfo *res = nullptr;
    std::string port_str = std::to_string(port);
    int gai = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (gai != 0)
        fail(std::string("getaddrinfo: ") + ::gai_strerror(gai));

    int raw_fd = -1;
    std::string last_err = "no address resolved";
    for (auto *ai = res; ai != nullptr; ai = ai->ai_next) {
        raw_fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (raw_fd < 0) {
            last_err = std::string("socket: ") + std::strerror(errno);
            continue;
        }
        set_nonblocking(raw_fd);

        int r = ::connect(raw_fd, ai->ai_addr, ai->ai_addrlen);
        if (r == 0)
            break;
        if (errno == EINPROGRESS) {
            wait_writable(raw_fd, CONNECT_TIMEOUT_MS);
            finalize_connect(raw_fd);
            break;
        }
        last_err = std::string("connect: ") + std::strerror(errno);
        ::close(raw_fd);
        raw_fd = -1;
    }
    ::freeaddrinfo(res);

    if (raw_fd < 0)
        fail(last_err);

    zappy::posix::FileDescriptor fd(raw_fd);
    _buffer = std::make_unique<zappy::net::ClientBuffer>(std::move(fd));
}

void ServerLink::send(std::string line)
{
    if (!_buffer)
        fail("send() on a closed link");
    _buffer->queue_message(std::move(line));
}

void ServerLink::poll_once(int timeout_ms)
{
    if (!_buffer || !_connected)
        return;

    short events = POLLIN;
    if (_buffer->has_pending_write())
        events |= POLLOUT;

    struct pollfd pfd{_buffer->fd(), events, 0};
    int r = ::poll(&pfd, 1, timeout_ms);
    if (r < 0) {
        if (errno == EINTR)
            return;
        fail_errno("poll");
    }
    if (r == 0)
        return;

    if (pfd.revents & POLLOUT) {
        if (_buffer->on_writable() == zappy::net::WriteResult::ERROR) {
            _connected = false;
            fail("write error");
        }
    }
    if (pfd.revents & POLLIN) {
        auto rr = _buffer->on_readable();
        if (rr == zappy::net::ReadResult::PEER_CLOSED) {
            _connected = false;
            fail("server closed the connection");
        }
        if (rr == zappy::net::ReadResult::ERROR) {
            _connected = false;
            fail("read error");
        }
        while (auto msg = _buffer->pop_ready_message())
            process_line(*msg);
    }
}

void ServerLink::send_bootstrap()
{
    _buffer->queue_message("GRAPHIC\n");
    _buffer->queue_message("msz\n");
    _buffer->queue_message("sgt\n");
    _buffer->queue_message("tna\n");
    _buffer->queue_message("mct\n");
}

void ServerLink::process_line(const std::string &line)
{
    if (_state == HandshakeState::WAIT_WELCOME) {
        if (line.rfind(std::string(WELCOME_LINE), 0) == 0) {
            send_bootstrap();
            _state = HandshakeState::READY;
        }
        return;
    }
    ServerEvent ev = ServerEventParser::parse(line);
    if (ev.kind != EventKind::UNKNOWN)
        _ready.push(std::move(ev));
}

std::optional<ServerEvent> ServerLink::next_message()
{
    if (_ready.empty())
        return std::nullopt;
    auto m = std::move(_ready.front());
    _ready.pop();
    return m;
}

bool ServerLink::is_connected() const noexcept
{
    return _connected;
}

} // namespace zappy::gui::net
