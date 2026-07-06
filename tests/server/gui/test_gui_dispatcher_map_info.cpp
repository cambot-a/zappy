/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for GUI map-info commands (msz, bct, mct, tna)
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
using zappy::server::game::Position;
using zappy::server::game::ResourceType;

namespace {

using Reply = std::pair<int, std::string>;

struct MapInfoFixture {
    World world;
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

    explicit MapInfoFixture(int w, int h,
        const std::vector<std::string> &teams = {"team"})
        : world(w, h, teams, 10)
    {
    }
};

} // namespace

Test(gui_map_info, msz_returns_correct_dimensions)
{
    MapInfoFixture fx(10, 10);
    fx.disp.dispatch(7, "msz");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "msz 10 10");
}

Test(gui_map_info, bct_on_empty_tile)
{
    MapInfoFixture fx(5, 5);
    fx.disp.dispatch(7, "bct 0 0");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "bct 0 0 0 0 0 0 0 0 0");
}

Test(gui_map_info, bct_with_resources)
{
    MapInfoFixture fx(5, 5);
    fx.world.setTileResource(Position(2, 3), ResourceType::FOOD, 5);
    fx.world.setTileResource(Position(2, 3), ResourceType::LINEMATE, 2);
    fx.disp.dispatch(7, "bct 2 3");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "bct 2 3 5 2 0 0 0 0 0");
}

Test(gui_map_info, bct_x_out_of_bounds)
{
    MapInfoFixture fx(5, 5);
    fx.disp.dispatch(7, "bct 5 0");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}

Test(gui_map_info, bct_y_out_of_bounds)
{
    MapInfoFixture fx(5, 5);
    fx.disp.dispatch(7, "bct 0 5");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}

Test(gui_map_info, bct_both_out_of_bounds)
{
    MapInfoFixture fx(5, 5);
    fx.disp.dispatch(7, "bct 10 10");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}

Test(gui_map_info, mct_returns_width_times_height_lines)
{
    MapInfoFixture fx(3, 3);
    fx.disp.dispatch(7, "mct");
    cr_assert_eq(fx.replies.size(), 9U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "bct 0 0 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[1].second.c_str(), "bct 1 0 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[2].second.c_str(), "bct 2 0 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[3].second.c_str(), "bct 0 1 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[4].second.c_str(), "bct 1 1 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[5].second.c_str(), "bct 2 1 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[6].second.c_str(), "bct 0 2 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[7].second.c_str(), "bct 1 2 0 0 0 0 0 0 0");
    cr_assert_str_eq(fx.replies[8].second.c_str(), "bct 2 2 0 0 0 0 0 0 0");
}

Test(gui_map_info, mct_content_reflects_actual_tiles)
{
    MapInfoFixture fx(3, 3);
    fx.world.setTileResource(Position(1, 2), ResourceType::SIBUR, 4);
    fx.disp.dispatch(7, "mct");
    cr_assert_eq(fx.replies.size(), 9U);
    cr_assert_str_eq(fx.replies[7].second.c_str(), "bct 1 2 0 0 0 4 0 0 0");
}

Test(gui_map_info, tna_with_multiple_teams)
{
    MapInfoFixture fx(5, 5, {"red", "blue", "green"});
    fx.disp.dispatch(7, "tna");
    cr_assert_eq(fx.replies.size(), 3U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "tna red");
    cr_assert_str_eq(fx.replies[1].second.c_str(), "tna blue");
    cr_assert_str_eq(fx.replies[2].second.c_str(), "tna green");
}

Test(gui_map_info, tna_with_one_team)
{
    MapInfoFixture fx(5, 5, {"solo"});
    fx.disp.dispatch(7, "tna");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "tna solo");
}
