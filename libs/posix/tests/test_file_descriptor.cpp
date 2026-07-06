/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Criterion tests for the FileDescriptor RAII owner
*/

#include <criterion/criterion.h>

#include <fcntl.h>
#include <unistd.h>

#include <type_traits>
#include <utility>

#include "posix/FileDescriptor.hpp"

static_assert(!std::is_copy_constructible_v<
    zappy::posix::FileDescriptor>, "must not be copyable");
static_assert(!std::is_copy_assignable_v<
    zappy::posix::FileDescriptor>, "must not be copy-assignable");
static_assert(std::is_move_constructible_v<
    zappy::posix::FileDescriptor>, "must be movable");
static_assert(std::is_move_assignable_v<
    zappy::posix::FileDescriptor>, "must be move-assignable");

namespace {

/// @brief Open a throwaway descriptor on /dev/null.
int open_fd()
{
    int fd = ::open("/dev/null", O_RDONLY);

    cr_assert_neq(fd, -1, "failed to open /dev/null");
    return fd;
}

/// @brief True if @p fd refers to an open descriptor.
bool fd_is_open(int fd)
{
    return ::fcntl(fd, F_GETFD) != -1;
}

} // namespace

Test(file_descriptor, default_is_invalid)
{
    zappy::posix::FileDescriptor fd;

    cr_assert_not(fd.is_valid());
    cr_assert_not(static_cast<bool>(fd));
    cr_assert_eq(fd.get(), -1);
}

Test(file_descriptor, takes_ownership_and_exposes_fd)
{
    int raw = open_fd();
    zappy::posix::FileDescriptor fd(raw);

    cr_assert(fd.is_valid());
    cr_assert(static_cast<bool>(fd));
    cr_assert_eq(fd.get(), raw);
}

Test(file_descriptor, destructor_closes_fd)
{
    int raw = open_fd();

    {
        zappy::posix::FileDescriptor fd(raw);
        cr_assert(fd_is_open(raw));
    }
    cr_assert_not(fd_is_open(raw), "descriptor must be closed after scope");
}

Test(file_descriptor, explicit_close_then_idempotent)
{
    int raw = open_fd();
    zappy::posix::FileDescriptor fd(raw);

    fd.close();
    cr_assert_not(fd_is_open(raw));
    cr_assert_not(fd.is_valid());
    fd.close();
    cr_assert_not(fd.is_valid(), "double close must be safe");
}

Test(file_descriptor, release_transfers_ownership)
{
    int raw = open_fd();
    zappy::posix::FileDescriptor fd(raw);

    int released = fd.release();

    cr_assert_eq(released, raw);
    cr_assert_not(fd.is_valid(), "owner must drop the fd after release");
    cr_assert(fd_is_open(raw), "released fd must stay open");
    ::close(released);
}

Test(file_descriptor, move_ctor_transfers)
{
    int raw = open_fd();
    zappy::posix::FileDescriptor src(raw);
    zappy::posix::FileDescriptor dst(std::move(src));

    cr_assert_eq(dst.get(), raw);
    cr_assert_not(src.is_valid(), "moved-from must be invalid");
    cr_assert(fd_is_open(raw));
}

Test(file_descriptor, move_assign_closes_previous)
{
    int first = open_fd();
    int second = open_fd();
    zappy::posix::FileDescriptor dst(first);
    zappy::posix::FileDescriptor src(second);

    dst = std::move(src);

    cr_assert_not(fd_is_open(first), "overwritten fd must be closed");
    cr_assert_eq(dst.get(), second);
    cr_assert_not(src.is_valid());
}
