/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Flood event implementation
*/

#include "server/event/Flood.hpp"
#include "server/game/World.hpp"

/**
 * @brief Build a flood over a bounding box.
 *
 * @param bboxOrigin top-left corner of the box on the map
 * @param bboxWidth box width in tiles
 * @param bboxHeight box height in tiles
 * @param durationTicks total lifetime in ticks
 */
zappy::server::event::Flood::Flood(game::Position bboxOrigin, int bboxWidth,
    int bboxHeight, int durationTicks) noexcept
    : _bboxOrigin(bboxOrigin), _bboxWidth(bboxWidth), _bboxHeight(bboxHeight),
      _durationTicks(durationTicks)
{
}

/**
 * @brief Protocol name of the event.
 *
 * @return std::string_view "flood"
 */
std::string_view zappy::server::event::Flood::name() const noexcept
{
    return "flood";
}

/**
 * @brief GUI line emitted when the flood starts.
 *
 * @return std::string "evt_flood_start <X> <Y> <W> <H>"
 */
std::string zappy::server::event::Flood::startBroadcast() const
{
    return "evt_flood_start " + std::to_string(_bboxOrigin.x()) + " "
        + std::to_string(_bboxOrigin.y()) + " " + std::to_string(_bboxWidth)
        + " " + std::to_string(_bboxHeight);
}

/**
 * @brief GUI line emitted on each active tick.
 *
 * @return std::string the empty string (no per-tick broadcast)
 */
std::string zappy::server::event::Flood::tickBroadcast() const
{
    return "";
}

/**
 * @brief GUI line emitted when the flood ends.
 *
 * @return std::string "evt_flood_end"
 */
std::string zappy::server::event::Flood::endBroadcast() const
{
    return "evt_flood_end";
}

/**
 * @brief Advance the flood one tick, marking the box on the first tick.
 *
 * @param world the world to mutate
 * @return bool true while active, false once expired
 */
bool zappy::server::event::Flood::applyTick(game::World &world)
{
    _elapsedTicks++;
    if (!_started) {
        floodTiles(world, true);
        _started = true;
    }
    return _elapsedTicks <= _durationTicks;
}

/**
 * @brief Wind the flood down, clearing every mark it set.
 *
 * @param world the world to restore
 */
void zappy::server::event::Flood::onEnd(game::World &world)
{
    if (_started)
        floodTiles(world, false);
}

/**
 * @brief Mark or clear every tile of the bounding box.
 *
 * @param world the world to mutate
 * @param value true to flood, false to dry out
 */
void zappy::server::event::Flood::floodTiles(game::World &world, bool value)
{
    const int width = world.width();
    const int height = world.height();

    for (int dy = 0; dy < _bboxHeight; dy++)
        for (int dx = 0; dx < _bboxWidth; dx++)
            world.setTileFlooded(game::Position(_bboxOrigin.x() + dx,
                _bboxOrigin.y() + dy).normalized(width, height), value);
}
