/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the EventBus infrastructure (ZAP-48, bonus)
*/

#include <criterion/criterion.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "server/event/Event.hpp"
#include "server/event/EventBus.hpp"
#include "server/config/FeatureFlag.hpp"
#include "server/config/FeatureFlags.hpp"
#include "server/client/ClientRegistry.hpp"
#include "server/game/World.hpp"
#include "server/gui/GuiNotifier.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::event::Event;
using zappy::server::event::EventBus;
using zappy::server::config::FeatureFlag;
using zappy::server::config::FeatureFlags;
using zappy::server::client::ClientRegistry;
using zappy::server::game::World;
using zappy::server::gui::GuiNotifier;
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

class TestEvent : public Event {
public:
    TestEvent(int ticksToLive, std::vector<int> &applyLog, int id) noexcept
        : _ticksToLive(ticksToLive), _applyLog(applyLog), _id(id) {}

    std::string_view name() const noexcept override { return "test"; }

    std::string startBroadcast() const override { return "evt_test_start"; }

    std::string endBroadcast() const override { return "evt_test_end"; }

    bool applyTick(World &) override
    {
        _applyLog.push_back(_id);
        return --_ticksToLive > 0;
    }

    void onEnd(World &) override { _applyLog.push_back(-_id); }

private:
    int _ticksToLive;
    std::vector<int> &_applyLog;
    int _id;
};

struct Fixture {
    World world{5, 5, {"team"}, 1};
    FeatureFlags flags;
    ClientRegistry clients;
    GuiNotifier notifier{clients, world, [](int) {}};
    FakeClock clock;
    Scheduler scheduler{clock};
    EventBus bus{world, notifier, scheduler, flags};
    std::vector<int> applyLog;

    void tickClock()
    {
        clock.advance(Duration(1000));
        scheduler.tick();
    }
};

} // namespace

Test(event_bus, initial_state_is_empty)
{
    Fixture fx;

    cr_assert_eq(fx.bus.activeEventCount(), 0U);
    cr_assert(fx.bus.activeEventNames().empty());
}

Test(event_bus, spawn_rejected_when_events_flag_off)
{
    Fixture fx;

    const bool accepted = fx.bus.spawn(
        std::make_unique<TestEvent>(5, fx.applyLog, 1));
    cr_assert(!accepted);
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
}

Test(event_bus, spawn_accepted_when_events_flag_on)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    const bool accepted = fx.bus.spawn(
        std::make_unique<TestEvent>(5, fx.applyLog, 1));
    cr_assert(accepted);
    cr_assert_eq(fx.bus.activeEventCount(), 1U);
}

Test(event_bus, apply_tick_runs_on_each_scheduler_tick)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.spawn(std::make_unique<TestEvent>(3, fx.applyLog, 7));
    fx.bus.start();
    fx.tickClock();
    cr_assert_eq(fx.applyLog.size(), 1U);
    fx.tickClock();
    cr_assert_eq(fx.applyLog.size(), 2U);
    fx.tickClock();
    cr_assert_eq(fx.applyLog.size(), 4U);
    cr_assert_eq(fx.applyLog[3], -7);
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
}

Test(event_bus, multiple_events_run_concurrently)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.spawn(std::make_unique<TestEvent>(2, fx.applyLog, 1));
    fx.bus.spawn(std::make_unique<TestEvent>(2, fx.applyLog, 2));
    fx.bus.start();
    fx.tickClock();
    cr_assert_eq(fx.applyLog.size(), 2U);
    cr_assert_eq(fx.applyLog[0], 1);
    cr_assert_eq(fx.applyLog[1], 2);
}

Test(event_bus, cancel_all_ends_every_event_instantly)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 1));
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 2));
    fx.bus.cancelAll();
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
    cr_assert_eq(fx.applyLog.size(), 2U);
    cr_assert_eq(fx.applyLog[0], -1);
    cr_assert_eq(fx.applyLog[1], -2);
}

Test(event_bus, flag_disabled_at_runtime_cancels_on_next_tick)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 1));
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 2));
    fx.bus.start();
    fx.flags.disable(FeatureFlag::EVENTS);
    fx.tickClock();
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
    cr_assert_eq(fx.applyLog.size(), 2U);
}

Test(event_bus, stop_cancels_tick_and_active_events)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 1));
    fx.bus.start();
    fx.bus.stop();
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
    fx.tickClock();
    cr_assert_eq(fx.applyLog.size(), 1U);
}

Test(event_bus, destructor_winds_down_active_events)
{
    Fixture fx;
    std::vector<int> log;

    fx.flags.enable(FeatureFlag::EVENTS);
    {
        EventBus localBus{
            fx.world, fx.notifier, fx.scheduler, fx.flags};
        localBus.spawn(std::make_unique<TestEvent>(99, log, 5));
        localBus.start();
    }
    cr_assert_eq(log.size(), 1U);
    cr_assert_eq(log[0], -5);
}

Test(event_bus, active_event_names_lists_active_events)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 1));
    fx.bus.spawn(std::make_unique<TestEvent>(99, fx.applyLog, 2));
    const auto names = fx.bus.activeEventNames();
    cr_assert_eq(names.size(), 2U);
    cr_assert(names[0] == "test");
    cr_assert(names[1] == "test");
}

Test(event_bus, spawn_null_event_is_rejected)
{
    Fixture fx;

    fx.flags.enable(FeatureFlag::EVENTS);
    cr_assert(!fx.bus.spawn(nullptr));
    cr_assert_eq(fx.bus.activeEventCount(), 0U);
}
