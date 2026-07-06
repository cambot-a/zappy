/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Clock abstraction for the scheduler time engine
*/

#ifndef SERVER_SCHEDULER_CLOCK_HPP_
    #define SERVER_SCHEDULER_CLOCK_HPP_

    #include <chrono>

namespace zappy::server::scheduler {

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

/**
 * @brief Source of monotonic time, injectable so tests can fake it.
 */
class IClock {
public:
    virtual ~IClock() = default;

    /**
     * @brief Current monotonic time point.
     *
     * @return TimePoint the current time
     */
    virtual TimePoint now() const noexcept = 0;
};

/**
 * @brief Production clock backed by std::chrono::steady_clock.
 */
class SteadyClock : public IClock {
public:
    /**
     * @brief Wall reading of the steady clock.
     *
     * @return TimePoint steady_clock::now()
     */
    TimePoint now() const noexcept override
    {
        return std::chrono::steady_clock::now();
    }
};

} // namespace zappy::server::scheduler

#endif /* !SERVER_SCHEDULER_CLOCK_HPP_ */
