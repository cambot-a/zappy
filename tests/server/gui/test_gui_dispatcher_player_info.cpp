/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for GUI player-info commands (ppo, plv, pin)
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
using zappy::server::game::Orientation;
using zappy::server::game::ResourceType;

namespace {

using Reply = std::pair<int, std::string>;

struct PlayerInfoFixture {
    World world{10, 10, {"team"}, 5};
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
    int id = world.addPlayer("team", Position(3, 4), Orientation::EAST);
};

} // namespace

Test(gui_player_info, ppo_on_existing_player)
{
    PlayerInfoFixture fx;
    fx.disp.dispatch(7, "ppo #" + std::to_string(fx.id));
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected = "ppo #" + std::to_string(fx.id) + " 3 4 2";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_player_info, ppo_on_non_existent_player)
{
    PlayerInfoFixture fx;
    fx.disp.dispatch(7, "ppo #999");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}

Test(gui_player_info, plv_on_existing_player)
{
    PlayerInfoFixture fx;
    fx.world.setPlayerLevel(fx.id, 3);
    fx.disp.dispatch(7, "plv #" + std::to_string(fx.id));
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected = "plv #" + std::to_string(fx.id) + " 3";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_player_info, plv_after_level_up)
{
    PlayerInfoFixture fx;
    fx.world.setPlayerLevel(fx.id, 5);
    fx.disp.dispatch(7, "plv #" + std::to_string(fx.id));
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected = "plv #" + std::to_string(fx.id) + " 5";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_player_info, pin_default_inventory)
{
    PlayerInfoFixture fx;
    fx.disp.dispatch(7, "pin #" + std::to_string(fx.id));
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected =
        "pin #" + std::to_string(fx.id) + " 3 4 10 0 0 0 0 0 0";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_player_info, pin_with_modified_inventory)
{
    PlayerInfoFixture fx;
    fx.world.player(fx.id).addResource(ResourceType::LINEMATE, 3);
    fx.world.player(fx.id).addResource(ResourceType::THYSTAME, 1);
    fx.disp.dispatch(7, "pin #" + std::to_string(fx.id));
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected =
        "pin #" + std::to_string(fx.id) + " 3 4 10 3 0 0 0 0 1";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_player_info, pin_on_non_existent_player)
{
    PlayerInfoFixture fx;
    fx.disp.dispatch(7, "pin #999");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}

Test(gui_player_info, ppo_reports_all_orientations)
{
    PlayerInfoFixture fx;
    const std::vector<std::pair<Orientation, char>> cases = {
        {Orientation::NORTH, '1'}, {Orientation::EAST, '2'},
        {Orientation::SOUTH, '3'}, {Orientation::WEST, '4'}};

    for (const auto &[orient, digit] : cases) {
        fx.replies.clear();
        fx.world.rotatePlayer(fx.id, orient);
        fx.disp.dispatch(7, "ppo #" + std::to_string(fx.id));
        cr_assert_eq(fx.replies.size(), 1U);
        cr_assert_eq(fx.replies[0].second.back(), digit);
    }
}

Test(gui_player_info, hash_prefix_is_optional_but_echoed)
{
    PlayerInfoFixture fx;
    fx.disp.dispatch(7, "ppo " + std::to_string(fx.id));
    cr_assert_eq(fx.replies.size(), 1U);
    const std::string expected = "ppo #" + std::to_string(fx.id) + " 3 4 2";
    cr_assert_str_eq(fx.replies[0].second.c_str(), expected.c_str());
}

Test(gui_player_info, negative_player_id_rejected_at_parse_time)
{
    PlayerInfoFixture fx;
    fx.disp.dispatch(7, "ppo #-1");
    cr_assert_eq(fx.replies.size(), 1U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "sbp");
}
