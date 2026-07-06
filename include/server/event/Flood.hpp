/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Flood natural event: marks a bounding box of tiles as flooded for a while
*/

#ifndef SERVER_EVENT_FLOOD_HPP_
    #define SERVER_EVENT_FLOOD_HPP_

    #include <string>
    #include <string_view>

    #include "server/event/Event.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {
class World;
} // namespace zappy::server::game

namespace zappy::server::event {

/**
 * @brief Timed flood marking every tile of a bounding box as flooded.
 *
 * On its first tick the flood marks every tile of the toroidal bounding box
 * @c (origin, width, height) as flooded; the marks persist until the event
 * expires after @c durationTicks ticks, at which point @ref onEnd clears them.
 * Flooded tiles block Forward and accelerate food consumption.
 */
class Flood : public Event {
public:
    /**
     * @brief Build a flood over a bounding box.
     *
     * @param bboxOrigin top-left corner of the box on the map
     * @param bboxWidth box width in tiles
     * @param bboxHeight box height in tiles
     * @param durationTicks total lifetime in ticks
     */
    Flood(game::Position bboxOrigin, int bboxWidth, int bboxHeight,
        int durationTicks) noexcept;

    /**
     * @brief Protocol name of the event.
     *
     * @return std::string_view "flood"
     */
    [[nodiscard]] std::string_view name() const noexcept override;

    /**
     * @brief GUI line emitted when the flood starts.
     *
     * @return std::string "evt_flood_start <X> <Y> <W> <H>"
     */
    [[nodiscard]] std::string startBroadcast() const override;

    /**
     * @brief GUI line emitted on each active tick.
     *
     * @return std::string the empty string (no per-tick broadcast)
     */
    [[nodiscard]] std::string tickBroadcast() const override;

    /**
     * @brief GUI line emitted when the flood ends.
     *
     * @return std::string "evt_flood_end"
     */
    [[nodiscard]] std::string endBroadcast() const override;

    /**
     * @brief Advance the flood one tick, marking the box on the first tick.
     *
     * @param world the world to mutate
     * @return bool true while active, false once expired
     */
    [[nodiscard]] bool applyTick(game::World &world) override;

    /**
     * @brief Wind the flood down, clearing every mark it set.
     *
     * @param world the world to restore
     */
    void onEnd(game::World &world) override;

private:
    /**
     * @brief Mark or clear every tile of the bounding box.
     *
     * @param world the world to mutate
     * @param value true to flood, false to dry out
     */
    void floodTiles(game::World &world, bool value);

    game::Position _bboxOrigin;
    int _bboxWidth;
    int _bboxHeight;
    int _durationTicks;
    int _elapsedTicks = 0;
    bool _started = false;
};

} // namespace zappy::server::event

#endif /* !SERVER_EVENT_FLOOD_HPP_ */
