/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the per-player food consumption loop
*/

#include <criterion/criterion.h>
#include <functional>
#include <vector>

#include "server/game/Constants.hpp"
#include "server/game/FoodScheduler.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::game::FoodScheduler;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;
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
    return World(5, 5, {"team"}, 10);
}

static int food(const World &world, int playerId)
{
    return world.player(playerId).resource(ResourceType::FOOD);
}

/* 1. frequency formula and defensive fallback */

Test(food_scheduler, interval_at_f_hundred_is_1260ms)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    FoodScheduler food(world, sched, 100, [](int) {});
    food.startConsumption(id);
    cr_assert_eq(sched.nextTimeoutMs(), 1260);
}

Test(food_scheduler, interval_at_f_one_is_126_seconds)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    FoodScheduler food(world, sched, 1, [](int) {});
    food.startConsumption(id);
    cr_assert_eq(sched.nextTimeoutMs(), 126000);
}

Test(food_scheduler, interval_at_f_zero_falls_back_to_126_seconds)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    FoodScheduler food(world, sched, 0, [](int) {});
    food.startConsumption(id);
    cr_assert_eq(sched.nextTimeoutMs(), 126000);
}

/* 2. food decreases over time until death */

Test(food_scheduler, food_decreases_then_player_dies)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    std::vector<int> deaths;
    FoodScheduler food(world, sched, 100,
        [&deaths](int dead) { deaths.push_back(dead); });
    food.startConsumption(id);
    for (int expected = 9; expected >= 0; --expected) {
        clock.advance(Duration(1260));
        sched.tick();
        cr_assert_eq(::food(world, id), expected);
    }
    cr_assert_eq(deaths.size(), 1U);
    cr_assert_eq(deaths.front(), id);
}

/* 3. stopConsumption cancels the pending event */

Test(food_scheduler, stop_cancels_consumption)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    std::vector<int> deaths;
    FoodScheduler food(world, sched, 100,
        [&deaths](int dead) { deaths.push_back(dead); });
    food.startConsumption(id);
    food.stopConsumption(id);
    clock.advance(Duration(1260));
    sched.tick();
    cr_assert_eq(::food(world, id), 10);
    cr_assert_eq(deaths.size(), 0U);
}

/* 4. stopConsumption on an unknown id is a no-op */

Test(food_scheduler, stop_unknown_id_is_noop)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    FoodScheduler food(world, sched, 100, [](int) {});
    food.stopConsumption(999);
    cr_assert_eq(sched.pendingCount(), 0U);
}

/* 5. multiple players consume independently */

Test(food_scheduler, players_consume_independently)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int a = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    const int b = world.addPlayer("team", Position(1, 1), Orientation::NORTH);
    FoodScheduler food(world, sched, 100, [](int) {});
    food.startConsumption(a);
    food.startConsumption(b);
    clock.advance(Duration(1260));
    sched.tick();
    cr_assert_eq(::food(world, a), 9);
    cr_assert_eq(::food(world, b), 9);
}

/* 6. death fires onDeath exactly once and unschedules */

Test(food_scheduler, death_fires_onDeath_only_once)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    world.player(id).removeResource(ResourceType::FOOD, 9);
    std::vector<int> deaths;
    FoodScheduler food(world, sched, 100,
        [&deaths](int dead) { deaths.push_back(dead); });
    food.startConsumption(id);
    clock.advance(Duration(1260));
    sched.tick();
    cr_assert_eq(deaths.size(), 1U);
    cr_assert_eq(sched.pendingCount(), 0U);
    clock.advance(Duration(1260));
    sched.tick();
    cr_assert_eq(deaths.size(), 1U);
}

/* 7. stopConsumption called from inside onDeath does not crash or reschedule */

Test(food_scheduler, stop_from_inside_onDeath_is_safe)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    world.player(id).removeResource(ResourceType::FOOD, 9);
    std::vector<int> deaths;
    FoodScheduler *self = nullptr;
    FoodScheduler food(world, sched, 100, [&](int dead) {
        deaths.push_back(dead);
        self->stopConsumption(dead);
    });
    self = &food;
    food.startConsumption(id);
    clock.advance(Duration(1260));
    sched.tick();
    cr_assert_eq(deaths.size(), 1U);
    cr_assert_eq(sched.pendingCount(), 0U);
}
