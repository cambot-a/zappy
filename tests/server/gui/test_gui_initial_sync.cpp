/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for GUI initial synchronization
*/

#include <criterion/criterion.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "server/game/World.hpp"
#include "server/gui/GuiDispatcher.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/event/EventBus.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::game::World;
using zappy::server::game::Position;
using zappy::server::game::Orientation;
using zappy::server::game::ResourceType;
using zappy::server::gui::GuiDispatcher;

struct SyncReply {
    int fd;
    std::string response;
};

struct SyncFixture {
    World world{2, 2, {"team1", "team2"}, 2};
    std::vector<SyncReply> replies;
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

Test(gui_initial_sync, sends_entire_state_in_order)
{
    SyncFixture f;

    // 1. Setup tile resources
    f.world.setTileResource(Position(0, 0), ResourceType::FOOD, 1);
    f.world.setTileResource(Position(0, 0), ResourceType::LINEMATE, 2);
    f.world.setTileResource(Position(0, 0), ResourceType::DERAUMERE, 0);
    f.world.setTileResource(Position(0, 0), ResourceType::SIBUR, 3);
    f.world.setTileResource(Position(0, 0), ResourceType::MENDIANE, 4);
    f.world.setTileResource(Position(0, 0), ResourceType::PHIRAS, 5);
    f.world.setTileResource(Position(0, 0), ResourceType::THYSTAME, 6);

    f.world.setTileResource(Position(1, 1), ResourceType::FOOD, 10);

    // 2. Setup players
    const int pid = f.world.addPlayer("team1", Position(1, 0), Orientation::EAST);
    f.world.setPlayerLevel(pid, 3);
    f.world.player(pid).addResource(ResourceType::FOOD, 5);
    f.world.player(pid).addResource(ResourceType::LINEMATE, 1);
    f.world.player(pid).addResource(ResourceType::DERAUMERE, 2);

    // 3. Setup eggs
    const int eggId = f.world.addEgg("team2", Position(0, 1));

    // 4. Trigger initial sync
    f.disp.sendInitialSync(42, 120);

    cr_assert_eq(f.replies.size(), 12U);

    // Verify all fds are 42
    for (const auto &reply : f.replies) {
        cr_assert_eq(reply.fd, 42);
    }

    // Check individual messages
    cr_assert_str_eq(f.replies[0].response.c_str(), "msz 2 2");

    cr_assert_str_eq(f.replies[1].response.c_str(), "bct 0 0 1 2 0 3 4 5 6");
    cr_assert_str_eq(f.replies[2].response.c_str(), "bct 1 0 0 0 0 0 0 0 0");
    cr_assert_str_eq(f.replies[3].response.c_str(), "bct 0 1 0 0 0 0 0 0 0");
    cr_assert_str_eq(f.replies[4].response.c_str(), "bct 1 1 10 0 0 0 0 0 0");

    cr_assert_str_eq(f.replies[5].response.c_str(), "tna team1");
    cr_assert_str_eq(f.replies[6].response.c_str(), "tna team2");

    // Connected player:
    // Newborn: pnw #n X Y O L N
    // Default food for new player is 10, we added 5 so total is 15.
    const std::string expectedPnw = "pnw " + std::to_string(pid) + " 1 0 2 3 team1";
    cr_assert_str_eq(f.replies[7].response.c_str(), expectedPnw.c_str());

    const std::string expectedPlv = "plv " + std::to_string(pid) + " 3";
    cr_assert_str_eq(f.replies[8].response.c_str(), expectedPlv.c_str());

    // Inventory: pin #n X Y q0 q1 q2 q3 q4 q5 q6
    const std::string expectedPin = "pin " + std::to_string(pid) + " 1 0 15 1 2 0 0 0 0";
    cr_assert_str_eq(f.replies[9].response.c_str(), expectedPin.c_str());

    // Egg:
    const std::string expectedEnw = "enw #" + std::to_string(eggId) + " -1 0 1";
    cr_assert_str_eq(f.replies[10].response.c_str(), expectedEnw.c_str());

    // Time:
    cr_assert_str_eq(f.replies[11].response.c_str(), "sgt 120");
}
