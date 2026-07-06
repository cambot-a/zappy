/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the ppf aggregated player profile command (ZAP-53, bonus)
*/

#include <criterion/criterion.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/game/World.hpp"
#include "server/event/EventBus.hpp"
#include "server/config/FeatureFlag.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::gui::GuiDispatcher;
using zappy::server::game::World;
using zappy::server::game::Position;
using zappy::server::game::Orientation;
using zappy::server::game::ResourceType;
using zappy::server::config::FeatureFlag;

namespace {

using Reply = std::pair<int, std::string>;

struct PpfFixture {
    World world{10, 10, {"alpha"}, 5};
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
    int id = world.addPlayer("alpha", Position(3, 4), Orientation::EAST);

    PpfFixture()
    {
        world.setPlayerLevel(id, 3);
        world.player(id).addResource(ResourceType::LINEMATE, 2);
    }

    std::string ppfCmd() const
    {
        return "ppf #" + std::to_string(id);
    }
};

std::size_t tokenCount(const std::string &line)
{
    return static_cast<std::size_t>(
        std::count(line.begin(), line.end(), ' ') + 1);
}

} // namespace

Test(gui_ppf, profile_off_returns_suc)
{
    PpfFixture fx;

    fx.flags.disable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "suc");
}

Test(gui_ppf, profile_on_existing_player_formatted_line)
{
    PpfFixture fx;

    fx.flags.enable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected =
        "ppf #" + std::to_string(fx.id) + " alpha 3 4 2 3 10 2 0 0 0 0 0";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_ppf, profile_on_unknown_player_returns_sbp)
{
    PpfFixture fx;

    fx.flags.enable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, "ppf #999");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}

Test(gui_ppf, inventory_change_reflected)
{
    PpfFixture fx;

    fx.flags.enable(FeatureFlag::PROFILE);
    fx.world.player(fx.id).addResource(ResourceType::THYSTAME, 1);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected =
        "ppf #" + std::to_string(fx.id) + " alpha 3 4 2 3 10 2 0 0 0 0 1";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_ppf, runtime_toggle)
{
    PpfFixture fx;

    fx.flags.enable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert(fx.replies.back().second.rfind("ppf #", 0) == 0);
    fx.flags.disable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert_str_eq(fx.replies.back().second.c_str(), "suc");
    fx.flags.enable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert(fx.replies.back().second.rfind("ppf #", 0) == 0);
}

Test(gui_ppf, no_admin_restriction)
{
    PpfFixture fx;

    fx.flags.enable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    fx.disp.dispatch(2, fx.ppfCmd());
    cr_assert_eq(fx.replies.size(), 2U);
    cr_assert_str_eq(fx.replies[0].second.c_str(),
        fx.replies[1].second.c_str());
}

Test(gui_ppf, line_has_fourteen_tokens)
{
    PpfFixture fx;

    fx.flags.enable(FeatureFlag::PROFILE);
    fx.disp.dispatch(1, fx.ppfCmd());
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_eq(tokenCount(fx.replies[0].second), 14U);
}
