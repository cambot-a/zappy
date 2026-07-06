/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Time-ordered event queue driving the server clock
*/

#ifndef SERVER_SCHEDULER_SCHEDULER_HPP_
    #define SERVER_SCHEDULER_SCHEDULER_HPP_

    #include <cstddef>
    #include <cstdint>
    #include <functional>
    #include <set>

    #include "server/scheduler/Clock.hpp"

namespace zappy::server::scheduler {

using EventId = std::uint64_t;

/**
 * @brief A single scheduled event; only `when` and `id` define its order.
 */
struct Event {
    EventId id;
    TimePoint when;
    std::function<void()> callback;
    mutable bool cancelled = false;
};

/**
 * @brief Orders events by fire time, breaking ties by insertion order (id).
 */
struct EventComparator {
    /**
     * @brief Strict-weak ordering on (when, id).
     *
     * @param lhs left operand
     * @param rhs right operand
     * @return bool true if lhs fires strictly before rhs
     */
    bool operator()(const Event &lhs, const Event &rhs) const noexcept
    {
        return lhs.when != rhs.when ? lhs.when < rhs.when : lhs.id < rhs.id;
    }
};

/**
 * @brief Holds pending timed callbacks and feeds poll() its next timeout.
 */
class Scheduler {
public:
    /**
     * @brief Build a scheduler reading time from @p clock (not owned).
     *
     * @param clock time source kept by reference
     */
    explicit Scheduler(IClock &clock) noexcept;

    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler &) = delete;
    Scheduler(Scheduler &&) = default;

    /**
     * @brief Schedule @p callback to fire after @p delay from now.
     *
     * @param delay wait before firing
     * @param callback function invoked when due
     * @return EventId the new event identifier
     */
    [[nodiscard]] EventId schedule(Duration delay,
        std::function<void()> callback);

    /**
     * @brief Schedule @p callback to fire at absolute time @p when.
     *
     * @param when absolute fire time
     * @param callback function invoked when due
     * @return EventId the new event identifier
     */
    [[nodiscard]] EventId scheduleAt(TimePoint when,
        std::function<void()> callback);

    /**
     * @brief Soft-cancel the event @p id; it stays queued but is skipped.
     *
     * @param id identifier returned by schedule
     * @return bool true if a live event was cancelled
     */
    bool cancel(EventId id) noexcept;

    /**
     * @brief Fire every due, non-cancelled event; never propagates exceptions.
     */
    void tick();

    /**
     * @brief Rescale the remaining delays of all active events by a factor.
     *
     * @param factor multiplier applied to remaining durations
     */
    void rescaleDelays(double factor) noexcept;

    /**
     * @brief Milliseconds until the next live event.
     *
     * @return int the delay, 0 if due, -1 if none
     */
    [[nodiscard]] int nextTimeoutMs() const noexcept;

    /**
     * @brief Number of live (non-cancelled) events.
     *
     * @return std::size_t the live count
     */
    [[nodiscard]] std::size_t pendingCount() const noexcept;

    /**
     * @brief Whether no live event remains.
     *
     * @return bool true if pendingCount() == 0
     */
    [[nodiscard]] bool empty() const noexcept;

private:
    /**
     * @brief Invoke @p callback, swallowing std::exception to stderr.
     *
     * @param callback function to run
     */
    void runCallback(const std::function<void()> &callback) noexcept;

    IClock &_clock;
    std::multiset<Event, EventComparator> _events;
    EventId _nextId = 1;
};

} // namespace zappy::server::scheduler

#endif /* !SERVER_SCHEDULER_SCHEDULER_HPP_ */
