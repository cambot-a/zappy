/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** EventBus implementation
*/

#include <algorithm>
#include <string>
#include <utility>

#include "server/event/EventBus.hpp"
#include "server/config/FeatureFlags.hpp"
#include "server/gui/GuiNotifier.hpp"

/**
 * @brief Wire the bus to the world, notifier, scheduler and flags.
 *
 * @param world world the events mutate
 * @param guiNotifier broadcaster for evt_* lifecycle lines
 * @param scheduler event queue driving the periodic tick
 * @param featureFlags flag registry gating event spawning
 */
zappy::server::event::EventBus::EventBus(
    game::World &world, gui::GuiNotifier &guiNotifier,
    scheduler::Scheduler &scheduler,
    const config::FeatureFlags &featureFlags) noexcept
    : _world(world), _guiNotifier(guiNotifier), _scheduler(scheduler),
      _featureFlags(featureFlags)
{
}

/**
 * @brief Cancel the tick and wind down every active event.
 */
zappy::server::event::EventBus::~EventBus() noexcept
{
    stop();
}

/**
 * @brief Schedule the recurring tick; idempotent.
 */
void zappy::server::event::EventBus::start()
{
    if (_tickEventId == 0)
        scheduleTick();
}

/**
 * @brief Cancel the recurring tick and all active events.
 */
void zappy::server::event::EventBus::stop() noexcept
{
    _scheduler.cancel(_tickEventId);
    _tickEventId = 0;
    cancelAll();
}

/**
 * @brief Take ownership of @p event and notify its start.
 *
 * @param event the event to activate
 * @return bool false (event dropped) if EVENTS is off or @p event is null
 */
bool zappy::server::event::EventBus::spawn(std::unique_ptr<Event> event)
{
    bool accepted = false;

    if (event && _featureFlags.isEnabled(config::FeatureFlag::EVENTS)) {
        notifyEventStarted(*event);
        _activeEvents.push_back(std::move(event));
        accepted = true;
    }
    return accepted;
}

/**
 * @brief End and remove every active event instantly.
 */
void zappy::server::event::EventBus::cancelAll()
{
    for (const auto &event : _activeEvents) {
        event->onEnd(_world);
        notifyEventEnded(*event);
    }
    _activeEvents.clear();
}

/**
 * @brief Number of currently active events.
 *
 * @return std::size_t the active count
 */
std::size_t zappy::server::event::EventBus::activeEventCount() const noexcept
{
    return _activeEvents.size();
}

/**
 * @brief Names of the currently active events, in activation order.
 *
 * @return std::vector<std::string_view> the active event names
 */
std::vector<std::string_view>
zappy::server::event::EventBus::activeEventNames() const
{
    std::vector<std::string_view> names;

    names.reserve(_activeEvents.size());
    for (const auto &event : _activeEvents)
        names.push_back(event->name());
    return names;
}

/**
 * @brief Fire the active events' tick, drop the expired, reschedule.
 */
void zappy::server::event::EventBus::onTick()
{
    if (!_featureFlags.isEnabled(config::FeatureFlag::EVENTS)) {
        cancelAll();
    } else {
        applyActiveEvents();
        scheduleTick();
    }
}

/**
 * @brief Tick each active event, ending and removing the expired ones.
 */
void zappy::server::event::EventBus::applyActiveEvents()
{
    std::erase_if(_activeEvents,
        [this](const std::unique_ptr<Event> &event) {
            const bool expired = !event->applyTick(_world);
            if (expired) {
                event->onEnd(_world);
                notifyEventEnded(*event);
            } else {
                notifyEventTicked(*event);
            }
            return expired;
        });
}

/**
 * @brief Schedule the next tick callback, storing its event id.
 */
void zappy::server::event::EventBus::scheduleTick()
{
    _tickEventId = _scheduler.schedule(tickInterval(),
        [this]() { onTick(); });
}

/**
 * @brief Delay between two ticks.
 *
 * @return scheduler::Duration the fixed one-second interval
 */
zappy::server::scheduler::Duration
zappy::server::event::EventBus::tickInterval() const noexcept
{
    return scheduler::Duration(1000);
}

/**
 * @brief Broadcast "evt_<name>_start" for @p event.
 *
 * @param event the event that started
 */
void zappy::server::event::EventBus::notifyEventStarted(const Event &event)
{
    _guiNotifier.broadcast(event.startBroadcast());
}

/**
 * @brief Broadcast the per-tick line for @p event when it is non-empty.
 *
 * @param event the event that just ticked
 */
void zappy::server::event::EventBus::notifyEventTicked(const Event &event)
{
    const std::string line = event.tickBroadcast();

    if (!line.empty())
        _guiNotifier.broadcast(line);
}

/**
 * @brief Broadcast "evt_<name>_end" for @p event.
 *
 * @param event the event that ended
 */
void zappy::server::event::EventBus::notifyEventEnded(const Event &event)
{
    _guiNotifier.broadcast(event.endBroadcast());
}
