/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Owns active natural events, ticking and broadcasting their lifecycle
*/

#ifndef SERVER_EVENT_EVENTBUS_HPP_
    #define SERVER_EVENT_EVENTBUS_HPP_

    #include <cstddef>
    #include <memory>
    #include <string_view>
    #include <vector>

    #include "server/event/Event.hpp"
    #include "server/scheduler/Scheduler.hpp"

namespace zappy::server::game {
class World;
} // namespace zappy::server::game

namespace zappy::server::gui {
class GuiNotifier;
} // namespace zappy::server::gui

namespace zappy::server::config {
class FeatureFlags;
} // namespace zappy::server::config

namespace zappy::server::event {

/**
 * @brief Owns active events, ticks them on the scheduler and broadcasts
 *        their start/end through the GUI notifier.
 *
 * Spawning is gated on the EVENTS feature flag; the bus always exists and is
 * inert while no event is active, so a runtime flag toggle needs no rebuild.
 */
class EventBus {
public:
    /**
     * @brief Wire the bus to the world, notifier, scheduler and flags.
     *
     * @param world world the events mutate
     * @param guiNotifier broadcaster for evt_* lifecycle lines
     * @param scheduler event queue driving the periodic tick
     * @param featureFlags flag registry gating event spawning
     */
    EventBus(game::World &world, gui::GuiNotifier &guiNotifier,
        scheduler::Scheduler &scheduler,
        const config::FeatureFlags &featureFlags) noexcept;

    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;
    EventBus(EventBus &&) = delete;
    EventBus &operator=(EventBus &&) = delete;

    /**
     * @brief Cancel the tick and wind down every active event.
     */
    ~EventBus() noexcept;

    /**
     * @brief Schedule the recurring tick; idempotent.
     */
    void start();

    /**
     * @brief Cancel the recurring tick and all active events.
     */
    void stop() noexcept;

    /**
     * @brief Take ownership of @p event and notify its start.
     *
     * @param event the event to activate
     * @return bool false (event dropped) if EVENTS is off or @p event is null
     */
    bool spawn(std::unique_ptr<Event> event);

    /**
     * @brief End and remove every active event instantly.
     */
    void cancelAll();

    /**
     * @brief Number of currently active events.
     *
     * @return std::size_t the active count
     */
    [[nodiscard]] std::size_t activeEventCount() const noexcept;

    /**
     * @brief Names of the currently active events, in activation order.
     *
     * @return std::vector<std::string_view> the active event names
     */
    [[nodiscard]] std::vector<std::string_view> activeEventNames() const;

private:
    /**
     * @brief Fire the active events' tick, drop the expired, reschedule.
     */
    void onTick();

    /**
     * @brief Tick each active event, ending and removing the expired ones.
     */
    void applyActiveEvents();

    /**
     * @brief Schedule the next tick callback, storing its event id.
     */
    void scheduleTick();

    /**
     * @brief Delay between two ticks.
     *
     * @return scheduler::Duration the fixed one-second interval
     */
    [[nodiscard]] scheduler::Duration tickInterval() const noexcept;

    /**
     * @brief Broadcast "evt_<name>_start" for @p event.
     *
     * @param event the event that started
     */
    void notifyEventStarted(const Event &event);

    /**
     * @brief Broadcast @p event's per-tick line if it is non-empty.
     *
     * @param event the event that just ticked
     */
    void notifyEventTicked(const Event &event);

    /**
     * @brief Broadcast "evt_<name>_end" for @p event.
     *
     * @param event the event that ended
     */
    void notifyEventEnded(const Event &event);

    game::World &_world;
    gui::GuiNotifier &_guiNotifier;
    scheduler::Scheduler &_scheduler;
    const config::FeatureFlags &_featureFlags;
    std::vector<std::unique_ptr<Event>> _activeEvents;
    scheduler::EventId _tickEventId = 0;
};

} // namespace zappy::server::event

#endif /* !SERVER_EVENT_EVENTBUS_HPP_ */
