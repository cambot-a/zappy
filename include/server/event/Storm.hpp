/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Storm natural event: periodically pushes players inside a circular zone
*/

#ifndef SERVER_EVENT_STORM_HPP_
    #define SERVER_EVENT_STORM_HPP_

    #include <string>
    #include <string_view>

    #include "server/event/Event.hpp"
    #include "server/game/Constants.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {
class World;
} // namespace zappy::server::game

namespace zappy::server::event {

/**
 * @brief Timed storm pushing every player inside a circular zone.
 *
 * Every @c pushIntervalTicks ticks the storm moves each non-dead player in the
 * affected radius one tile along its direction, wrapping on the torus. It
 * broadcasts evt_storm_start once, evt_storm_tick every tick and evt_storm_end
 * on expiry, after @c durationTicks ticks.
 */
class Storm : public Event {
public:
    /**
     * @brief Build a storm centred on @p center.
     *
     * @param center zone centre on the map
     * @param radius zone radius in tiles
     * @param direction push direction applied to caught players
     * @param pushIntervalTicks ticks between two pushes
     * @param durationTicks total lifetime in ticks
     */
    Storm(game::Position center, int radius, game::Orientation direction,
        int pushIntervalTicks, int durationTicks) noexcept;

    /**
     * @brief Protocol name of the event.
     *
     * @return std::string_view "storm"
     */
    [[nodiscard]] std::string_view name() const noexcept override;

    /**
     * @brief GUI line emitted when the storm starts.
     *
     * @return std::string "evt_storm_start <X> <Y> <R> <O>"
     */
    [[nodiscard]] std::string startBroadcast() const override;

    /**
     * @brief GUI line emitted on each active tick.
     *
     * @return std::string "evt_storm_tick <X> <Y> <R> <O>"
     */
    [[nodiscard]] std::string tickBroadcast() const override;

    /**
     * @brief GUI line emitted when the storm ends.
     *
     * @return std::string "evt_storm_end"
     */
    [[nodiscard]] std::string endBroadcast() const override;

    /**
     * @brief Advance the storm one tick, pushing players on push ticks.
     *
     * @param world the world to mutate
     * @return bool true while active, false once expired
     */
    [[nodiscard]] bool applyTick(game::World &world) override;

    /**
     * @brief Wind the storm down; nothing to restore.
     *
     * @param world the world (unused)
     */
    void onEnd(game::World &world) override;

    /**
     * @brief Zone centre.
     *
     * @return game::Position the centre
     */
    [[nodiscard]] game::Position center() const noexcept;

    /**
     * @brief Zone radius in tiles.
     *
     * @return int the radius
     */
    [[nodiscard]] int radius() const noexcept;

    /**
     * @brief Push direction.
     *
     * @return game::Orientation the direction
     */
    [[nodiscard]] game::Orientation direction() const noexcept;

private:
    /**
     * @brief Shared "<X> <Y> <R> <O>" payload for the broadcast lines.
     *
     * @return std::string the formatted zone arguments
     */
    [[nodiscard]] std::string zoneArguments() const;

    /**
     * @brief Whether @p pos lies within the toroidal radius of the centre.
     *
     * @param pos position to test
     * @param worldWidth map width
     * @param worldHeight map height
     * @return bool true if squared distance <= radius squared
     */
    [[nodiscard]] bool isInZone(game::Position pos, int worldWidth,
        int worldHeight) const noexcept;

    /**
     * @brief Push every caught, non-dead player one tile along the direction.
     *
     * @param world the world to mutate
     */
    void pushPlayersInZone(game::World &world);

    game::Position _center;
    int _radius;
    game::Orientation _direction;
    int _pushIntervalTicks;
    int _durationTicks;
    int _elapsedTicks = 0;
};

} // namespace zappy::server::event

#endif /* !SERVER_EVENT_STORM_HPP_ */
