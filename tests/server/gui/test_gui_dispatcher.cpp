/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the GUI command dispatcher
*/

#include <criterion/criterion.h>
#include <string>
#include <utility>
#include <vector>

#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/game/World.hpp"
#include "server/event/EventBus.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::gui::GuiDispatcher;
using zappy::server::game::World;

namespace {

using Reply = std::pair<int, std::string>;

struct Fixture {
    World world{10, 10, {"team"}, 10};
    std::vector<Reply> replies;
    int frequency = 100;
    zappy::server::client::ClientRegistry clients;
    zappy::server::config::FeatureFlags flags;
    zappy::server::scheduler::SteadyClock clock;
    zappy::server::scheduler::Scheduler scheduler{clock};
    zappy::server::gui::GuiNotifier notifier{clients, world, [](int) {}};
    zappy::server::event::EventBus eventBus{
        world, notifier, scheduler, flags};
    GuiDispatcher disp{world,
        zappy::server::gui::GuiAdminDeps{clients, flags, "", {}, eventBus},
        [this](int fd, std::string r) {
            replies.push_back({fd, std::move(r)});
        },
        [this] { return frequency; },
        [this](int f) { frequency = f; }
    };
};

} // namespace

Test(gui_dispatcher, unknown_command_returns_suc)
{
    Fixture fx;
    fx.disp.dispatch(42, "Bogus");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_eq(fx.replies[0].first, 42);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "suc");
}

Test(gui_dispatcher, bad_parameter_returns_sbp)
{
    Fixture fx;
    fx.disp.dispatch(42, "bct 1");
    fx.disp.dispatch(42, "ppo abc");
    cr_assert_eq(fx.replies.size(), 2U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
    cr_assert_str_eq(fx.replies[1].second.c_str(), "sbp");
}

Test(gui_dispatcher, map_info_commands_produce_real_output)
{
    Fixture fx;
    fx.disp.dispatch(42, "msz");
    cr_assert_str_eq(fx.replies.back().second.c_str(), "msz 10 10");
    fx.disp.dispatch(42, "bct 3 5");
    cr_assert_str_eq(fx.replies.back().second.c_str(), "bct 3 5 0 0 0 0 0 0 0");
    fx.disp.dispatch(42, "tna");
    cr_assert_str_eq(fx.replies.back().second.c_str(), "tna team");
}

Test(gui_dispatcher, time_management_commands)
{
    Fixture fx;
    fx.disp.dispatch(42, "sgt");
    cr_assert_str_eq(fx.replies.back().second.c_str(), "sgt 100");

    fx.disp.dispatch(42, "sst 200");
    cr_assert_eq(fx.frequency, 200);

    fx.disp.dispatch(42, "sst 0");
    cr_assert_str_eq(fx.replies.back().second.c_str(), "sbp");
}

Test(gui_dispatcher, dispatch_is_instant)
{
    Fixture fx;
    cr_assert_eq(fx.replies.size(), 0U);
    fx.disp.dispatch(100, "msz");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_eq(fx.replies[0].first, 100);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "msz 10 10");
}
