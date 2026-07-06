/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Inventory command tests for the AI dispatcher
*/

#include <criterion/criterion.h>
#include <string>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/Constants.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

/**
 * @brief Test clock whose time only moves when the test advances it.
 */
class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

/**
 * @brief Flags any mutation notification the world may emit.
 */
class MockObserver : public WorldObserverAdapter {
public:
    void onTileChanged(Position) override { tileChanged = true; }
    void onPlayerMoved(int, Position, Position) override { moved = true; }
    void onPlayerRotated(int) override { rotated = true; }
    void onPlayerInventoryChanged(int) override { inventoryChanged = true; }
    void onPlayerStateChanged(int) override { stateChanged = true; }

    bool tileChanged = false;
    bool moved = false;
    bool rotated = false;
    bool inventoryChanged = false;
    bool stateChanged = false;
};

} // namespace

/* 1. default inventory reports the starting food and zeroed stones */

Test(ai_dispatcher_inventory, default_inventory)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(std::move(r)); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Inventory");
    clock.advance(Duration(10));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(),
        "[food 10, linemate 0, deraumere 0, sibur 0, mendiane 0, "
        "phiras 0, thystame 0]");
}

/* 2. added resources are reflected in the reported inventory */

Test(ai_dispatcher_inventory, modified_inventory)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(std::move(r)); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    world.player(id).addResource(ResourceType::LINEMATE, 3);
    world.player(id).addResource(ResourceType::THYSTAME, 1);
    world.player(id).addResource(ResourceType::FOOD, 5);
    disp.dispatch(id, "Inventory");
    clock.advance(Duration(10));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(),
        "[food 15, linemate 3, deraumere 0, sibur 0, mendiane 0, "
        "phiras 0, thystame 1]");
}

/* 3. inventory fires exactly at 1/f, not a millisecond before */

Test(ai_dispatcher_inventory, duration_one_over_f)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(std::move(r)); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Inventory");
    clock.advance(Duration(9));
    sched.tick();
    cr_assert(responses.empty());
    clock.advance(Duration(1));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_eq(responses.front().compare(0, 6, "[food "), 0);
}

/* 4. response is bracketed with exactly six separators between seven entries */

Test(ai_dispatcher_inventory, format_brackets_and_separators)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(std::move(r)); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Inventory");
    clock.advance(Duration(10));
    sched.tick();
    const std::string &response = responses.front();
    std::size_t separators = 0;
    for (std::size_t pos = response.find(", "); pos != std::string::npos;
        pos = response.find(", ", pos + 1))
        ++separators;
    cr_assert_eq(response.front(), '[');
    cr_assert_eq(response.back(), ']');
    cr_assert_eq(separators, 6U);
}

/* 5. inventory is read-only and emits no mutation notification */

Test(ai_dispatcher_inventory, observer_no_change)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(std::move(r)); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    MockObserver obs;
    world.addObserver(obs);
    disp.dispatch(id, "Inventory");
    clock.advance(Duration(10));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert(!obs.tileChanged);
    cr_assert(!obs.moved);
    cr_assert(!obs.rotated);
    cr_assert(!obs.inventoryChanged);
    cr_assert(!obs.stateChanged);
}
