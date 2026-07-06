/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the time-ordered event scheduler
*/

#include <criterion/criterion.h>
#include <stdexcept>
#include <vector>

#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::scheduler::Duration;
using zappy::server::scheduler::EventId;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

/**
 * @brief Test clock whose time only moves when the test advances it.
 */
class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    void setNow(TimePoint t) noexcept { _now = t; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

static Duration ms(int value)
{
    return Duration(value);
}

/* 1. empty scheduler */

Test(scheduler, empty_has_no_timeout_and_no_events)
{
    FakeClock clock;
    Scheduler sched(clock);
    cr_assert_eq(sched.nextTimeoutMs(), -1);
    cr_assert_eq(sched.pendingCount(), 0U);
    cr_assert(sched.empty());
    sched.tick();
    cr_assert_eq(sched.pendingCount(), 0U);
}

/* 2. single future event */

Test(scheduler, single_event_countdown_then_fire)
{
    FakeClock clock;
    Scheduler sched(clock);
    bool fired = false;
    (void)sched.schedule(ms(100), [&fired]{ fired = true; });
    cr_assert_eq(sched.nextTimeoutMs(), 100);
    clock.advance(ms(50));
    cr_assert_eq(sched.nextTimeoutMs(), 50);
    clock.advance(ms(50));
    cr_assert_eq(sched.nextTimeoutMs(), 0);
    sched.tick();
    cr_assert(fired);
    cr_assert(sched.empty());
}

/* 3. multiple events fire in time order */

Test(scheduler, events_fire_in_time_order)
{
    FakeClock clock;
    Scheduler sched(clock);
    std::vector<int> order;
    (void)sched.schedule(ms(30), [&order]{ order.push_back(30); });
    (void)sched.schedule(ms(10), [&order]{ order.push_back(10); });
    (void)sched.schedule(ms(20), [&order]{ order.push_back(20); });
    clock.advance(ms(100));
    sched.tick();
    cr_assert_eq(order.size(), 3U);
    cr_assert_eq(order[0], 10);
    cr_assert_eq(order[1], 20);
    cr_assert_eq(order[2], 30);
}

/* 4. simultaneous events keep insertion order */

Test(scheduler, simultaneous_events_keep_insertion_order)
{
    FakeClock clock;
    Scheduler sched(clock);
    std::vector<int> order;
    (void)sched.schedule(ms(10), [&order]{ order.push_back(1); });
    (void)sched.schedule(ms(10), [&order]{ order.push_back(2); });
    (void)sched.schedule(ms(10), [&order]{ order.push_back(3); });
    clock.advance(ms(10));
    sched.tick();
    cr_assert_eq(order.size(), 3U);
    cr_assert_eq(order[0], 1);
    cr_assert_eq(order[1], 2);
    cr_assert_eq(order[2], 3);
}

/* 5. partial tick fires only due events */

Test(scheduler, partial_tick_leaves_future_event)
{
    FakeClock clock;
    Scheduler sched(clock);
    int fired = 0;
    (void)sched.schedule(ms(10), [&fired]{ ++fired; });
    (void)sched.schedule(ms(100), [&fired]{ ++fired; });
    clock.advance(ms(50));
    sched.tick();
    cr_assert_eq(fired, 1);
    cr_assert_eq(sched.pendingCount(), 1U);
    cr_assert_eq(sched.nextTimeoutMs(), 50);
}

/* 6. cancel before firing */

Test(scheduler, cancel_prevents_firing)
{
    FakeClock clock;
    Scheduler sched(clock);
    bool fired = false;
    const EventId id = sched.schedule(ms(10), [&fired]{ fired = true; });
    cr_assert(sched.cancel(id));
    cr_assert_not(sched.cancel(id));
    cr_assert_not(sched.cancel(9999));
    clock.advance(ms(50));
    sched.tick();
    cr_assert_not(fired);
}

/* 7. cancel returns false on already-fired event */

Test(scheduler, cancel_on_fired_event_returns_false)
{
    FakeClock clock;
    Scheduler sched(clock);
    const EventId id = sched.schedule(ms(10), []{});
    clock.advance(ms(10));
    sched.tick();
    cr_assert_not(sched.cancel(id));
}

/* 8. throwing callback does not break the tick */

Test(scheduler, throwing_callback_does_not_stop_others)
{
    FakeClock clock;
    Scheduler sched(clock);
    bool second = false;
    (void)sched.schedule(ms(10), []{ throw std::runtime_error("boom"); });
    (void)sched.schedule(ms(20), [&second]{ second = true; });
    clock.advance(ms(50));
    sched.tick();
    cr_assert(second);
    cr_assert(sched.empty());
}

/* 9. nextTimeoutMs skips cancelled head */

Test(scheduler, next_timeout_skips_cancelled_head)
{
    FakeClock clock;
    Scheduler sched(clock);
    const EventId a = sched.schedule(ms(10), []{});
    (void)sched.schedule(ms(20), []{});
    cr_assert(sched.cancel(a));
    cr_assert_eq(sched.nextTimeoutMs(), 20);
    cr_assert_eq(sched.pendingCount(), 1U);
}

/* 10. reschedule from inside a callback */

Test(scheduler, callback_can_reschedule)
{
    FakeClock clock;
    Scheduler sched(clock);
    int fired = 0;
    (void)sched.schedule(ms(10), [&]{
        ++fired;
        (void)sched.schedule(ms(10), [&fired]{ ++fired; });
    });
    clock.advance(ms(10));
    sched.tick();
    cr_assert_eq(fired, 1);
    clock.advance(ms(10));
    sched.tick();
    cr_assert_eq(fired, 2);
}

Test(scheduler, rescale_delays_adjusts_timeouts)
{
    FakeClock clock;
    Scheduler sched(clock);
    int fired_count = 0;
    (void)sched.schedule(ms(100), [&fired_count]{ ++fired_count; });

    clock.advance(ms(20));
    cr_assert_eq(sched.nextTimeoutMs(), 80);

    sched.rescaleDelays(0.5);
    cr_assert_eq(sched.nextTimeoutMs(), 40);

    clock.advance(ms(20));
    cr_assert_eq(sched.nextTimeoutMs(), 20);

    sched.rescaleDelays(2.0);
    cr_assert_eq(sched.nextTimeoutMs(), 40);

    clock.advance(ms(40));
    cr_assert_eq(sched.nextTimeoutMs(), 0);
    sched.tick();
    cr_assert_eq(fired_count, 1);
}
