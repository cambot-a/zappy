/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the AI command dispatcher
*/

#include <criterion/criterion.h>
#include <string>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
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

static World makeWorld()
{
    World world(5, 5, {"team"}, 10);
    world.addPlayer("team", Position(2, 2), Orientation::NORTH);
    return world;
}

/* 1. unknown command -> immediate ko */

Test(ai_dispatcher, unknown_command_responds_ko_immediately)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Bogus");
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ko");
    cr_assert_eq(sched.pendingCount(), 0U);
}

/* 2. valid command schedules then fires */

Test(ai_dispatcher, valid_command_fires_after_its_duration)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Forward");
    cr_assert_eq(responses.size(), 0U);
    cr_assert_eq(sched.nextTimeoutMs(), 70);
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ok");
}

/* 3. two commands run sequentially */

Test(ai_dispatcher, two_commands_run_back_to_back)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Forward");
    disp.dispatch(1, "Right");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_eq(sched.pendingCount(), 1U);
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 2U);
}

/* 4. queue caps at 10 accepted commands */

Test(ai_dispatcher, queue_caps_at_ten_commands)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    for (int i = 0; i < 12; ++i)
        disp.dispatch(1, "Forward");
    for (int i = 0; i < 12; ++i) {
        clock.advance(Duration(70));
        sched.tick();
    }
    cr_assert_eq(responses.size(), 10U);
}

/* 5. inventory uses its own (shorter) duration */

Test(ai_dispatcher, inventory_has_one_time_unit_duration)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Inventory");
    cr_assert_eq(sched.nextTimeoutMs(), 10);
    clock.advance(Duration(10));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
}

/* 6. missing required argument -> immediate ko */

Test(ai_dispatcher, missing_argument_responds_ko_immediately)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Take");
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ko");
    cr_assert_eq(sched.pendingCount(), 0U);
}

/* 7. argument command is accepted and fires */

Test(ai_dispatcher, broadcast_with_argument_fires)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Broadcast hello world");
    cr_assert_eq(sched.nextTimeoutMs(), 70);
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ok");
}

/* 8. stopPlayer cancels everything */

Test(ai_dispatcher, stop_player_cancels_in_flight_and_queue)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Forward");
    disp.dispatch(1, "Right");
    disp.dispatch(1, "Left");
    disp.stopPlayer(1);
    cr_assert_eq(sched.pendingCount(), 0U);
    clock.advance(Duration(1000));
    sched.tick();
    cr_assert_eq(responses.size(), 0U);
}

/* 9. stopPlayer is idempotent and leaves a fresh slate */

Test(ai_dispatcher, stop_player_is_idempotent)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.stopPlayer(999);
    disp.stopPlayer(1);
    disp.dispatch(1, "Forward");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "ok");
}

/* 10. zero-duration commands execute without advancing the clock */

Test(ai_dispatcher, connect_nbr_executes_synchronously)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Connect_nbr");
    disp.dispatch(1, "Connect_nbr");
    disp.dispatch(1, "Connect_nbr");
    cr_assert_eq(responses.size(), 3U);
    cr_assert_eq(sched.pendingCount(), 0U);
}

Test(ai_dispatcher, zero_duration_commands_drain_in_one_tick)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    disp.dispatch(1, "Forward");
    disp.dispatch(1, "Connect_nbr");
    disp.dispatch(1, "Connect_nbr");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 3U);
    cr_assert_eq(sched.pendingCount(), 0U);
}
