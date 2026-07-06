/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the GUI admin authentication command (ZAP-46, bonus)
*/

#include <criterion/criterion.h>
#include <string>
#include <utility>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>

#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/game/World.hpp"
#include "server/client/ClientState.hpp"
#include "server/config/FeatureFlag.hpp"
#include "server/event/EventBus.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"
#include "posix/FileDescriptor.hpp"

using zappy::server::gui::GuiDispatcher;
using zappy::server::gui::GuiAdminDeps;
using zappy::server::gui::GuiNotifier;
using zappy::server::game::World;
using zappy::server::client::Client;
using zappy::server::client::ClientRegistry;
using zappy::server::client::ClientState;
using zappy::server::config::FeatureFlags;
using zappy::server::config::FeatureFlag;
using zappy::server::event::EventBus;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::SteadyClock;
using zappy::posix::FileDescriptor;

namespace {

using Reply = std::pair<int, std::string>;

struct AdminFixture {
    World world{10, 10, {"team"}, 5};
    ClientRegistry clients;
    FeatureFlags flags;
    std::vector<Reply> replies;
    int frequency = 100;
    SteadyClock clock;
    Scheduler scheduler{clock};
    GuiNotifier notifier{clients, world, [](int) {}};
    EventBus eventBus{world, notifier, scheduler, flags};
    GuiDispatcher disp;

    AdminFixture(std::string password, bool adminEnabled)
        : disp(world,
            GuiAdminDeps{clients, flags, std::move(password), {}, eventBus},
            [this](int fd, std::string r) {
                replies.push_back({fd, std::move(r)});
            },
            [this] { return frequency; },
            [this](int f) { frequency = f; })
    {
        if (adminEnabled)
            flags.enable(FeatureFlag::ADMIN);
    }

    int addClient(ClientState state)
    {
        int sv[2];

        socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        close(sv[1]);
        Client &c = clients.add(FileDescriptor{sv[0]});
        c.promote(state);
        return c.fd();
    }
};

} // namespace

Test(gui_dispatcher_admin, flag_off_returns_suc_and_keeps_gui_state)
{
    AdminFixture fx{"secret", false};
    const int fd = fx.addClient(ClientState::GUI);

    fx.disp.dispatch(fd, "admin secret");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "suc");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI);
}

Test(gui_dispatcher_admin, correct_password_promotes_and_returns_ok)
{
    AdminFixture fx{"secret", true};
    const int fd = fx.addClient(ClientState::GUI);

    fx.disp.dispatch(fd, "admin secret");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ok");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI_ADMIN);
}

Test(gui_dispatcher_admin, wrong_password_returns_ko_and_keeps_gui_state)
{
    AdminFixture fx{"secret", true};
    const int fd = fx.addClient(ClientState::GUI);

    fx.disp.dispatch(fd, "admin wrong");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ko");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI);
}

Test(gui_dispatcher_admin, empty_password_input_returns_sbp)
{
    AdminFixture fx{"secret", true};
    const int fd = fx.addClient(ClientState::GUI);

    fx.disp.dispatch(fd, "admin");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI);
}

Test(gui_dispatcher_admin, password_with_spaces_is_matched)
{
    AdminFixture fx{"my secret", true};
    const int fd = fx.addClient(ClientState::GUI);

    fx.disp.dispatch(fd, "admin my secret");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ok");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI_ADMIN);
}

Test(gui_dispatcher_admin, already_admin_correct_password_stays_admin)
{
    AdminFixture fx{"secret", true};
    const int fd = fx.addClient(ClientState::GUI_ADMIN);

    fx.disp.dispatch(fd, "admin secret");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ok");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI_ADMIN);
}

Test(gui_dispatcher_admin, already_admin_wrong_password_no_downgrade)
{
    AdminFixture fx{"secret", true};
    const int fd = fx.addClient(ClientState::GUI_ADMIN);

    fx.disp.dispatch(fd, "admin wrong");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ko");
    cr_assert(fx.clients.get(fd).state() == ClientState::GUI_ADMIN);
}

/* Defensive: the server never routes AI clients to the GUI dispatcher.
** If it ever does, an AI client must not be promoted: handleAdmin returns
** "ko" (no eligible GUI state) and the client stays in AI state. */
Test(gui_dispatcher_admin, ai_client_is_not_promoted)
{
    AdminFixture fx{"secret", true};
    const int fd = fx.addClient(ClientState::AI);

    fx.disp.dispatch(fd, "admin secret");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ko");
    cr_assert(fx.clients.get(fd).state() == ClientState::AI);
}
