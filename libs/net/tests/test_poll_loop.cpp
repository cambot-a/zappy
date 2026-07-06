/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Criterion tests for the poll() event loop
*/

#include <criterion/criterion.h>

#include <signal.h>
#include <unistd.h>

#include "net/PollLoop.hpp"

namespace {

/// @brief Create a pipe and write one byte so the read-end is poll-ready.
void ready_pipe(int fds[2])
{
    const char byte = 1;

    cr_assert_eq(::pipe(fds), 0, "pipe() failed");
    cr_assert_eq(::write(fds[1], &byte, 1), 1);
}

/// @brief SIGALRM handler raising SIGINT to interrupt a blocking poll.
void raise_sigint_on_alarm(int)
{
    ::raise(SIGINT);
}

} // namespace

Test(poll_loop, stop_before_run_returns_immediately, .timeout = 2)
{
    zappy::net::PollLoop loop;

    loop.stop();
    loop.run();
    cr_assert(true, "run() must return after stop()");
}

Test(poll_loop, ready_fd_invokes_callback, .timeout = 2)
{
    zappy::net::PollLoop loop;
    int fds[2];
    int calls = 0;

    ready_pipe(fds);
    loop.register_fd(fds[0], POLLIN, [&](short) {
        ++calls;
        loop.stop();
    });
    loop.run();
    cr_assert_eq(calls, 1, "callback must fire exactly once");
    ::close(fds[0]);
    ::close(fds[1]);
}

Test(poll_loop, unregister_removes_fd, .timeout = 2)
{
    zappy::net::PollLoop loop;
    int a[2];
    int b[2];
    int a_calls = 0;
    int b_calls = 0;

    ready_pipe(a);
    ready_pipe(b);
    loop.register_fd(a[0], POLLIN, [&](short) {
        ++a_calls;
        loop.stop();
    });
    loop.register_fd(b[0], POLLIN, [&](short) { ++b_calls; });
    loop.unregister_fd(b[0]);
    loop.run();
    cr_assert_eq(a_calls, 1);
    cr_assert_eq(b_calls, 0, "unregistered fd must not be polled");
    ::close(a[0]);
    ::close(a[1]);
    ::close(b[0]);
    ::close(b[1]);
}

Test(poll_loop, modify_events_enables_polling, .timeout = 2)
{
    zappy::net::PollLoop loop;
    int fds[2];
    int calls = 0;

    ready_pipe(fds);
    loop.register_fd(fds[0], 0, [&](short) {
        ++calls;
        loop.stop();
    });
    loop.modify_events(fds[0], POLLIN);
    loop.run();
    cr_assert_eq(calls, 1, "modify_events must enable the callback");
    ::close(fds[0]);
    ::close(fds[1]);
}

Test(poll_loop, sigint_during_poll_stops_loop, .timeout = 3)
{
    zappy::net::PollLoop loop;

    signal(SIGALRM, raise_sigint_on_alarm);
    alarm(1);
    loop.run();
    cr_assert(true, "SIGINT delivered during poll must end run()");
}
