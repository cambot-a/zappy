/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Look command tests for the AI dispatcher
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
using zappy::server::game::ResourceType;
using zappy::server::game::World;
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

} // namespace

/* 1. empty world, level 1, facing north */

Test(ai_dispatcher_look, empty_world_level_one)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Look");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
    cr_assert_str_eq(responses.front().c_str(), "[player, , , ]");
}

/* 2. one food tile ahead */

Test(ai_dispatcher_look, food_one_tile_ahead)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    world.setTileResource(Position(5, 4), ResourceType::FOOD, 1);
    disp.dispatch(id, "Look");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(), "[player, , food, ]");
}

/* 3. multiple resources on one tile */

Test(ai_dispatcher_look, multiple_resources_one_tile)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    world.setTileResource(Position(5, 4), ResourceType::FOOD, 1);
    world.setTileResource(Position(5, 4), ResourceType::LINEMATE, 2);
    disp.dispatch(id, "Look");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(),
        "[player, , food linemate linemate, ]");
}

/* 4. multiple players on the requester tile */

Test(ai_dispatcher_look, multiple_players_same_tile)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Look");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(), "[player player, , , ]");
}

/* 5. level 2 vision yields nine tiles */

Test(ai_dispatcher_look, level_two_vision)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    world.setPlayerLevel(id, 2);
    disp.dispatch(id, "Look");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(),
        "[player, , , , , , , , ]");
}

/* 6. wrapped tile shows its resource */

Test(ai_dispatcher_look, wrap_visible_in_response)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 0), Orientation::NORTH);
    world.setTileResource(Position(5, 9), ResourceType::FOOD, 1);
    disp.dispatch(id, "Look");
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_str_eq(responses.front().c_str(), "[player, , food, ]");
}

/* 7. look fires after 7/f, not before */

Test(ai_dispatcher_look, look_duration_is_seven_over_f)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(10, 10, {"team"}, 10);
    std::vector<std::string> responses;
    AiDispatcher disp(world, sched, 100,
        [&responses](int, std::string r) { responses.push_back(r); });
    const int id = world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    disp.dispatch(id, "Look");
    cr_assert_eq(sched.nextTimeoutMs(), 70);
    cr_assert_eq(responses.size(), 0U);
    clock.advance(Duration(70));
    sched.tick();
    cr_assert_eq(responses.size(), 1U);
}
