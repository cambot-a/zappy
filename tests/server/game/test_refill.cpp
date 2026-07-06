/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the resource refill loop
*/

#include <criterion/criterion.h>
#include <array>
#include <cstddef>

#include "server/game/Constants.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/RefillScheduler.hpp"
#include "server/game/ResourceSpawner.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::game::Position;
using zappy::server::game::RefillScheduler;
using zappy::server::game::ResourceSpawner;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::game::RESOURCE_COUNT;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

static const std::array<int, RESOURCE_COUNT> kTargets10x10 =
    {50, 30, 15, 10, 10, 8, 5};

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
 * @brief Counts onTileChanged notifications received from the world.
 */
class TileCounter : public WorldObserverAdapter {
public:
    int tileChanges = 0;

    void onTileChanged(Position) override { ++tileChanges; }
};

static World makeWorld(int width = 10, int height = 10)
{
    return World(width, height, {"alpha", "beta"}, 2);
}

static int totalOnMap(const World &world, ResourceType type)
{
    int total = 0;
    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            total += world.tileAt(Position(x, y)).resource(type);
    return total;
}

/* 1. refill fills an empty map up to target */

Test(refill_missing, fills_empty_map_to_target)
{
    World world = makeWorld();
    ResourceSpawner spawner(42);
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        cr_assert_eq(totalOnMap(world, static_cast<ResourceType>(i)), 0);
    spawner.refillMissing(world);
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        cr_assert_eq(totalOnMap(world, static_cast<ResourceType>(i)),
            kTargets10x10[i]);
}

/* 2. refill never removes a surplus and never over-adds */

Test(refill_missing, leaves_full_map_untouched)
{
    World world = makeWorld();
    ResourceSpawner spawner(42);
    spawner.spawnInitial(world);
    std::array<int, RESOURCE_COUNT> before{};
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        before[i] = totalOnMap(world, static_cast<ResourceType>(i));
    spawner.refillMissing(world);
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        cr_assert_eq(totalOnMap(world, static_cast<ResourceType>(i)),
            before[i]);
}

/* 3. refill tops up a partially depleted resource only */

Test(refill_missing, tops_up_depleted_food_only)
{
    World world = makeWorld();
    ResourceSpawner spawner(42);
    spawner.spawnInitial(world);
    std::array<int, RESOURCE_COUNT> before{};
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        before[i] = totalOnMap(world, static_cast<ResourceType>(i));
    int drained = totalOnMap(world, ResourceType::FOOD) - 25;
    for (int y = 0; y < world.height() && drained > 0; ++y)
        for (int x = 0; x < world.width() && drained > 0; ++x) {
            const Position pos(x, y);
            int here = world.tileAt(pos).resource(ResourceType::FOOD);
            int take = here < drained ? here : drained;
            world.setTileResource(pos, ResourceType::FOOD, here - take);
            drained -= take;
        }
    cr_assert_eq(totalOnMap(world, ResourceType::FOOD), 25);
    spawner.refillMissing(world);
    cr_assert_eq(totalOnMap(world, ResourceType::FOOD), 50);
    for (std::size_t i = 1; i < RESOURCE_COUNT; ++i)
        cr_assert_eq(totalOnMap(world, static_cast<ResourceType>(i)),
            before[i]);
}

/* 4. refill fires onTileChanged once per added unit */

Test(refill_missing, fires_one_event_per_added_unit)
{
    World world = makeWorld();
    TileCounter counter;
    world.addObserver(counter);
    ResourceSpawner spawner(42);
    spawner.refillMissing(world);
    cr_assert_eq(counter.tileChanges, 128);
}

/* 5. scheduler schedules and reschedules the refill */

Test(refill_scheduler, schedules_and_reschedules)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    ResourceSpawner spawner(42);
    RefillScheduler refill(world, spawner, sched, 100);
    refill.start();
    cr_assert_eq(sched.nextTimeoutMs(), 200);
    clock.advance(Duration(200));
    sched.tick();
    cr_assert_eq(totalOnMap(world, ResourceType::FOOD), 50);
    cr_assert_eq(sched.pendingCount(), 1U);
    cr_assert_eq(sched.nextTimeoutMs(), 200);
    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            world.setTileResource(Position(x, y), ResourceType::FOOD, 0);
    cr_assert_eq(totalOnMap(world, ResourceType::FOOD), 0);
    clock.advance(Duration(200));
    sched.tick();
    cr_assert_eq(totalOnMap(world, ResourceType::FOOD), 50);
    refill.stop();
    cr_assert_eq(sched.pendingCount(), 0U);
}

/* 6. stop cancels the pending event before it fires */

Test(refill_scheduler, stop_cancels_pending_event)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    ResourceSpawner spawner(42);
    RefillScheduler refill(world, spawner, sched, 100);
    refill.start();
    refill.stop();
    clock.advance(Duration(200));
    sched.tick();
    cr_assert_eq(totalOnMap(world, ResourceType::FOOD), 0);
    cr_assert(sched.empty());
}

/* 7. destructor cancels the pending event */

Test(refill_scheduler, destructor_calls_stop)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    ResourceSpawner spawner(42);
    {
        RefillScheduler refill(world, spawner, sched, 100);
        refill.start();
        cr_assert_eq(sched.pendingCount(), 1U);
    }
    cr_assert_eq(sched.pendingCount(), 0U);
}

/* 8. frequency formula and defensive fallback */

Test(refill_scheduler, interval_formula)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    ResourceSpawner spawner(42);
    RefillScheduler fast(world, spawner, sched, 100);
    fast.start();
    cr_assert_eq(sched.nextTimeoutMs(), 200);
}

Test(refill_scheduler, interval_at_f_one_is_twenty_seconds)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    ResourceSpawner spawner(42);
    RefillScheduler slow(world, spawner, sched, 1);
    slow.start();
    cr_assert_eq(sched.nextTimeoutMs(), 20000);
}

Test(refill_scheduler, interval_at_f_zero_falls_back_to_twenty_seconds)
{
    FakeClock clock;
    Scheduler sched(clock);
    World world = makeWorld();
    ResourceSpawner spawner(42);
    RefillScheduler guarded(world, spawner, sched, 0);
    guarded.start();
    cr_assert_eq(sched.nextTimeoutMs(), 20000);
}
