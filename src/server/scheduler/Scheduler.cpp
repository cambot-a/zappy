/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Time-ordered event queue driving the server clock
*/

#include "server/scheduler/Scheduler.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>

zappy::server::scheduler::Scheduler::Scheduler(IClock &clock) noexcept
    : _clock(clock)
{}

zappy::server::scheduler::EventId
zappy::server::scheduler::Scheduler::schedule(
    Duration delay, std::function<void()> callback)
{
    return scheduleAt(_clock.now() + delay, std::move(callback));
}

zappy::server::scheduler::EventId
zappy::server::scheduler::Scheduler::scheduleAt(
    TimePoint when, std::function<void()> callback)
{
    const EventId id = _nextId++;
    _events.insert(Event{id, when, std::move(callback), false});
    return id;
}

bool zappy::server::scheduler::Scheduler::cancel(EventId id) noexcept
{
    const auto it = std::find_if(_events.begin(), _events.end(),
        [id](const Event &e) noexcept {
            return e.id == id && !e.cancelled;
        });
    const bool found = it != _events.end();
    if (found)
        it->cancelled = true;
    return found;
}

void zappy::server::scheduler::Scheduler::tick()
{
    while (!_events.empty() && _events.begin()->when <= _clock.now()) {
        const Event due = *_events.begin();
        _events.erase(_events.begin());
        if (!due.cancelled)
            runCallback(due.callback);
    }
}

int zappy::server::scheduler::Scheduler::nextTimeoutMs() const noexcept
{
    const auto it = std::find_if(_events.begin(), _events.end(),
        [](const Event &e) noexcept { return !e.cancelled; });
    int result = -1;
    if (it != _events.end()) {
        const auto delta = std::chrono::duration_cast<Duration>(
            it->when - _clock.now());
        result = static_cast<int>(std::max<std::int64_t>(0, delta.count()));
    }
    return result;
}

std::size_t zappy::server::scheduler::Scheduler::pendingCount() const noexcept
{
    return static_cast<std::size_t>(std::count_if(_events.begin(),
        _events.end(),
        [](const Event &e) noexcept { return !e.cancelled; }));
}

bool zappy::server::scheduler::Scheduler::empty() const noexcept
{
    return pendingCount() == 0;
}

void zappy::server::scheduler::Scheduler::runCallback(
    const std::function<void()> &callback) noexcept
{
    try {
        callback();
    } catch (const std::exception &e) {
        std::cerr << "scheduler callback error: " << e.what() << "\n";
    }
}

/**
 * @brief Rescale the remaining delays of all active events by a factor.
 *
 * @param factor multiplier applied to remaining durations
 */
void zappy::server::scheduler::Scheduler::rescaleDelays(
    double factor) noexcept
{
    const TimePoint now = _clock.now();
    std::multiset<Event, EventComparator> newEvents;

    for (const auto &event : _events) {
        if (event.cancelled)
            continue;
        Duration remaining = std::chrono::duration_cast<Duration>(
            event.when - now);
        if (remaining < Duration::zero())
            remaining = Duration::zero();
        double ms = static_cast<double>(remaining.count()) * factor;
        Duration newRemaining(static_cast<Duration::rep>(ms));
        Event updated = event;
        updated.when = now + newRemaining;
        newEvents.insert(updated);
    }
    _events = std::move(newEvents);
}
