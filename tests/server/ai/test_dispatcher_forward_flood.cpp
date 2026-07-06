/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Forward-into-flooded-tile tests for the AI dispatcher (ZAP-50, bonus)
*/

#include <criterion/criterion.h>
#include <string>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

} // namespace

Test(dispatcher_forward_flood, forward_into_flooded_tile_returns_ko)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);

    world.setTileFlooded(Position(5, 4), true);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ko");
    cr_assert(world.player(id).position() == Position(5, 5));
}

Test(dispatcher_forward_flood, forward_into_dried_tile_returns_ok)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);

    world.setTileFlooded(Position(5, 4), true);
    world.setTileFlooded(Position(5, 4), false);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(), "ok");
    cr_assert(world.player(id).position() == Position(5, 4));
}

Test(dispatcher_forward_flood, forward_from_flooded_tile_is_allowed)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);

    world.setTileFlooded(Position(5, 5), true);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(), "ok");
    cr_assert(world.player(id).position() == Position(5, 4));
}
