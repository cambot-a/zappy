/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Per-player recurring food consumption and starvation signalling
*/

#ifndef SERVER_GAME_FOODSCHEDULER_HPP_
    #define SERVER_GAME_FOODSCHEDULER_HPP_

    #include <functional>
    #include <unordered_map>

    #include "server/game/World.hpp"
    #include "server/scheduler/Clock.hpp"
    #include "server/scheduler/Scheduler.hpp"

namespace zappy::server::game {

/**
 * @brief Drives per-player food consumption; signals starvation via a callback.
 */
class FoodScheduler {
public:
    /**
     * @brief Wire the consumption loop without scheduling anything yet.
     *
     * @param world world holding the players
     * @param scheduler event queue owning the timers
     * @param frequency reciprocal of the time unit (the f config value)
     * @param onDeath callback invoked with the playerId when food hits 0
     */
    FoodScheduler(World &world, scheduler::Scheduler &scheduler,
        int frequency, std::function<void(int)> onDeath);

    FoodScheduler(const FoodScheduler &) = delete;
    FoodScheduler &operator=(const FoodScheduler &) = delete;
    FoodScheduler(FoodScheduler &&) = delete;
    FoodScheduler &operator=(FoodScheduler &&) = delete;

    /**
     * @brief Cancel every pending consumption event defensively.
     */
    ~FoodScheduler() noexcept;

    /**
     * @brief Schedule the first consumption for @p playerId; idempotent.
     *
     * @param playerId player to start consuming food
     */
    void startConsumption(int playerId);

    /**
     * @brief Cancel the pending consumption for @p playerId; no-op if unknown.
     *
     * @param playerId player to stop consuming food
     */
    void stopConsumption(int playerId) noexcept;

    /**
     * @brief Update the time frequency.
     *
     * @param frequency new time frequency
     */
    void setFrequency(int frequency) noexcept;

private:
    /**
     * @brief Delay before @p playerId next consumes food, in milliseconds.
     *
     * A player standing on a flooded tile consumes food
     * @c FOOD_FLOOD_MULTIPLIER times faster.
     *
     * @param playerId player whose tile state drives the interval
     * @return scheduler::Duration the consumption interval
     */
    [[nodiscard]] scheduler::Duration consumptionInterval(
        int playerId) const noexcept;

    /**
     * @brief Consume one food unit; reschedule or signal starvation.
     *
     * @param playerId player consuming food
     */
    void onConsume(int playerId);

    World &_world;
    scheduler::Scheduler &_scheduler;
    int _frequency;
    std::function<void(int)> _onDeath;
    std::unordered_map<int, scheduler::EventId> _events;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_FOODSCHEDULER_HPP_ */
