/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Flood-accelerated food consumption tests (ZAP-50, bonus)
*/

#include <criterion/criterion.h>

#include "server/game/Constants.hpp"
#include "server/game/FoodScheduler.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::game::FoodScheduler;
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

Test(food_scheduler_flood, normal_rate_on_dry_tile)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(5, 5, {"team"}, 10);
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    FoodScheduler food(world, sched, 100, [](int) {});

    food.startConsumption(id);
    cr_assert_eq(sched.nextTimeoutMs(), 1260);
}

Test(food_scheduler_flood, triple_rate_on_flooded_tile)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(5, 5, {"team"}, 10);
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    FoodScheduler food(world, sched, 100, [](int) {});

    world.setTileFlooded(Position(0, 0), true);
    food.startConsumption(id);
    cr_assert_eq(sched.nextTimeoutMs(), 420);
}

Test(food_scheduler_flood, leaving_flooded_tile_restores_normal_rate)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world(5, 5, {"team"}, 10);
    const int id = world.addPlayer("team", Position(0, 0), Orientation::NORTH);
    FoodScheduler food(world, sched, 100, [](int) {});

    world.setTileFlooded(Position(0, 0), true);
    food.startConsumption(id);
    cr_assert_eq(sched.nextTimeoutMs(), 420);
    world.setTileFlooded(Position(0, 0), false);
    clock.advance(Duration(420));
    sched.tick();
    cr_assert_eq(sched.nextTimeoutMs(), 1260);
}
