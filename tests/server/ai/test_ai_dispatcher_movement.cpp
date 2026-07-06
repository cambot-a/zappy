/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Movement command tests for the AI dispatcher (Forward, Right, Left)
*/

#include <criterion/criterion.h>
#include <string>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
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
 * @brief Records the last move and rotation notifications.
 */
class MockObserver : public WorldObserverAdapter {
public:
    void onPlayerMoved(int, Position oldPos, Position newPos) override
    {
        moved = true;
        lastOld = oldPos;
        lastNew = newPos;
    }

    void onPlayerRotated(int) override { rotated = true; }

    bool moved = false;
    bool rotated = false;
    Position lastOld{};
    Position lastNew{};
};

} // namespace

/* 1. forward facing north decreases y */

Test(ai_dispatcher_movement, forward_north_moves_up)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(world.player(id).position() == Position(5, 4));
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ok");
}

/* 2. forward facing east increases x */

Test(ai_dispatcher_movement, forward_east_moves_right)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::EAST);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(world.player(id).position() == Position(6, 5));
}

/* 3. forward wraps over the top edge */

Test(ai_dispatcher_movement, forward_wraps_top_edge)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 0), Orientation::NORTH);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(world.player(id).position() == Position(5, 9));
}

/* 4. forward wraps over the right edge */

Test(ai_dispatcher_movement, forward_wraps_right_edge)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(9, 5), Orientation::EAST);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(world.player(id).position() == Position(0, 5));
}

/* 5. right turns clockwise without moving */

Test(ai_dispatcher_movement, right_rotates_clockwise)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Right");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(world.player(id).orientation(), Orientation::EAST);
    cr_assert(world.player(id).position() == Position(5, 5));
    for (int i = 0; i < 3; ++i) {
        disp.dispatch(id, "Right");
        clock.advance(Duration(70));
        sched.tick();
    }
    cr_assert_eq(world.player(id).orientation(), Orientation::NORTH);
}

/* 6. left turns counter-clockwise without moving */

Test(ai_dispatcher_movement, left_rotates_counter_clockwise)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Left");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(world.player(id).orientation(), Orientation::WEST);
    cr_assert(world.player(id).position() == Position(5, 5));
    for (int i = 0; i < 3; ++i) {
        disp.dispatch(id, "Left");
        clock.advance(Duration(70));
        sched.tick();
    }
    cr_assert_eq(world.player(id).orientation(), Orientation::NORTH);
}

/* 7. right then forward goes east */

Test(ai_dispatcher_movement, right_then_forward_goes_east)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Right");
    clock.advance(Duration(70));
    sched.tick();
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(world.player(id).position() == Position(6, 5));
    cr_assert_eq(world.player(id).orientation(), Orientation::EAST);
}

/* 8. forward notifies observers of the move */

Test(ai_dispatcher_movement, forward_fires_on_player_moved)
{
    FakeClock clock;
    Scheduler sched(clock);
    MockObserver obs;
    World world(10, 10, {"team"}, 10);
    world.addObserver(obs);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(obs.moved);
    cr_assert(obs.lastOld == Position(5, 5));
    cr_assert(obs.lastNew == Position(5, 4));
}

/* 9. right notifies observers of the rotation */

Test(ai_dispatcher_movement, right_fires_on_player_rotated)
{
    FakeClock clock;
    Scheduler sched(clock);
    MockObserver obs;
    World world(10, 10, {"team"}, 10);
    world.addObserver(obs);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Right");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert(obs.rotated);
}
