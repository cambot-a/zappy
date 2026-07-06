/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Recurring resource refill driven by the scheduler
*/

#ifndef SERVER_GAME_REFILLSCHEDULER_HPP_
    #define SERVER_GAME_REFILLSCHEDULER_HPP_

    #include "server/game/ResourceSpawner.hpp"
    #include "server/game/World.hpp"
    #include "server/scheduler/Clock.hpp"
    #include "server/scheduler/Scheduler.hpp"

namespace zappy::server::game {

/**
 * @brief Owns the recurring refill event: decides when ResourceSpawner refills.
 */
class RefillScheduler {
public:
    /**
     * @brief Wire the refill loop without scheduling anything yet.
     *
     * @param world world to keep stocked
     * @param spawner spawner doing the actual refill
     * @param scheduler event queue owning the timer
     * @param frequency reciprocal of the time unit (the f config value)
     */
    RefillScheduler(World &world, ResourceSpawner &spawner,
        scheduler::Scheduler &scheduler, int frequency) noexcept;

    RefillScheduler(const RefillScheduler &) = delete;
    RefillScheduler &operator=(const RefillScheduler &) = delete;
    RefillScheduler(RefillScheduler &&) = delete;
    RefillScheduler &operator=(RefillScheduler &&) = delete;

    /**
     * @brief Cancel any pending refill event defensively.
     */
    ~RefillScheduler() noexcept;

    /**
     * @brief Schedule the first refill; idempotent once started.
     */
    void start();

    /**
     * @brief Cancel the pending refill event if any; safe to call anytime.
     */
    void stop() noexcept;

    /**
     * @brief Update the time frequency.
     *
     * @param frequency new time frequency
     */
    void setFrequency(int frequency) noexcept;

private:
    /**
     * @brief Delay between two refills in milliseconds.
     *
     * @return scheduler::Duration the refill interval
     */
    [[nodiscard]] scheduler::Duration refillInterval() const noexcept;

    /**
     * @brief Refill the world then reschedule the next refill.
     */
    void onRefill();

    World &_world;
    ResourceSpawner &_spawner;
    scheduler::Scheduler &_scheduler;
    int _frequency;
    scheduler::EventId _currentEventId = 0;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_REFILLSCHEDULER_HPP_ */
