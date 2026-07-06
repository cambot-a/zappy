/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Per-player recurring food consumption and starvation signalling
*/

#include <utility>

#include "server/game/FoodScheduler.hpp"

zappy::server::game::FoodScheduler::FoodScheduler(World &world,
    scheduler::Scheduler &scheduler, int frequency,
    std::function<void(int)> onDeath)
    : _world(world), _scheduler(scheduler), _frequency(frequency),
      _onDeath(std::move(onDeath))
{}

zappy::server::game::FoodScheduler::~FoodScheduler() noexcept
{
    for (const auto &[playerId, eventId] : _events)
        _scheduler.cancel(eventId);
}

void zappy::server::game::FoodScheduler::startConsumption(int playerId)
{
    if (_events.find(playerId) == _events.end())
        _events[playerId] = _scheduler.schedule(consumptionInterval(playerId),
            [this, playerId] { onConsume(playerId); });
}

void zappy::server::game::FoodScheduler::stopConsumption(int playerId) noexcept
{
    const auto it = _events.find(playerId);
    if (it != _events.end()) {
        _scheduler.cancel(it->second);
        _events.erase(it);
    }
}

zappy::server::scheduler::Duration
zappy::server::game::FoodScheduler::consumptionInterval(
    int playerId) const noexcept
{
    int divisor = 1;

    if (_world.hasPlayer(playerId)
        && _world.tileAt(_world.player(playerId).position()).isFlooded())
        divisor = FOOD_FLOOD_MULTIPLIER;
    return std::chrono::milliseconds(_frequency > 0
        ? FOOD_CONSUMPTION_INTERVAL_TIME_UNITS * 1000 / (_frequency * divisor)
        : FOOD_CONSUMPTION_INTERVAL_TIME_UNITS * 1000 / divisor);
}

void zappy::server::game::FoodScheduler::onConsume(int playerId)
{
    if (!_world.hasPlayer(playerId)) {
        _events.erase(playerId);
        return;
    }
    _world.consumePlayerFood(playerId);
    if (_world.player(playerId).resource(ResourceType::FOOD) > 0) {
        _events[playerId] = _scheduler.schedule(consumptionInterval(playerId),
            [this, playerId] { onConsume(playerId); });
    } else {
        _events.erase(playerId);
        _onDeath(playerId);
    }
}

/**
 * @brief Update the time frequency.
 *
 * @param frequency new time frequency
 */
void zappy::server::game::FoodScheduler::setFrequency(
    int frequency) noexcept
{
    _frequency = frequency;
}
