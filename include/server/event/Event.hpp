/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Abstract interface for a natural event applied to the world over time
*/

#ifndef SERVER_EVENT_EVENT_HPP_
    #define SERVER_EVENT_EVENT_HPP_

    #include <string>
    #include <string_view>

namespace zappy::server::game {
class World;
} // namespace zappy::server::game

namespace zappy::server::event {

/**
 * @brief A timed natural event owning its own state and world effects.
 *
 * Concrete events (Storm, Flood, Meteor) implement this interface; they hold
 * their internal timers and affected regions, apply effects on each tick and
 * restore the world when they end.
 */
class Event {
public:
    virtual ~Event() = default;

    /**
     * @brief Protocol name of the event (e.g. "storm"); drives evt_* lines.
     *
     * @return std::string_view the lowercase event name
     */
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    /**
     * @brief GUI line broadcast once when the event starts.
     *
     * @return std::string the "evt_<name>_start ..." line
     */
    [[nodiscard]] virtual std::string startBroadcast() const = 0;

    /**
     * @brief GUI line broadcast on each active tick; empty means none.
     *
     * @return std::string the per-tick line, or "" for no broadcast
     */
    [[nodiscard]] virtual std::string tickBroadcast() const { return ""; }

    /**
     * @brief GUI line broadcast once when the event ends.
     *
     * @return std::string the "evt_<name>_end ..." line
     */
    [[nodiscard]] virtual std::string endBroadcast() const = 0;

    /**
     * @brief Apply one tick of the event's effect to @p world.
     *
     * @param world the world to mutate
     * @return bool true to stay active, false once the event has expired
     */
    [[nodiscard]] virtual bool applyTick(game::World &world) = 0;

    /**
     * @brief Wind the event down, restoring any world state it altered.
     *
     * Called once when the event expires naturally or is cancelled.
     *
     * @param world the world to restore
     */
    virtual void onEnd(game::World &world) = 0;
};

} // namespace zappy::server::event

#endif /* !SERVER_EVENT_EVENT_HPP_ */
