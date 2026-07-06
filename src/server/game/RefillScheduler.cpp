/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Recurring resource refill driven by the scheduler
*/

#include "server/game/RefillScheduler.hpp"

zappy::server::game::RefillScheduler::RefillScheduler(World &world,
    ResourceSpawner &spawner, scheduler::Scheduler &scheduler,
    int frequency) noexcept
    : _world(world), _spawner(spawner), _scheduler(scheduler),
      _frequency(frequency)
{}

zappy::server::game::RefillScheduler::~RefillScheduler() noexcept
{
    stop();
}

void zappy::server::game::RefillScheduler::start()
{
    if (_currentEventId == 0)
        _currentEventId = _scheduler.schedule(
            refillInterval(), [this] { onRefill(); });
}

void zappy::server::game::RefillScheduler::stop() noexcept
{
    if (_currentEventId != 0) {
        _scheduler.cancel(_currentEventId);
        _currentEventId = 0;
    }
}

zappy::server::scheduler::Duration
zappy::server::game::RefillScheduler::refillInterval() const noexcept
{
    return std::chrono::milliseconds(
        _frequency > 0 ? 20 * 1000 / _frequency : 20000);
}

void zappy::server::game::RefillScheduler::onRefill()
{
    _spawner.refillMissing(_world);
    _currentEventId = _scheduler.schedule(
        refillInterval(), [this] { onRefill(); });
}

/**
 * @brief Update the time frequency.
 *
 * @param frequency new time frequency
 */
void zappy::server::game::RefillScheduler::setFrequency(
    int frequency) noexcept
{
    _frequency = frequency;
}
