/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Storm event implementation
*/

#include "server/event/Storm.hpp"
#include "server/game/OrientationHelper.hpp"
#include "server/game/Player.hpp"
#include "server/game/World.hpp"

/**
 * @brief Build a storm centred on @p center.
 *
 * @param center zone centre on the map
 * @param radius zone radius in tiles
 * @param direction push direction applied to caught players
 * @param pushIntervalTicks ticks between two pushes
 * @param durationTicks total lifetime in ticks
 */
zappy::server::event::Storm::Storm(game::Position center, int radius,
    game::Orientation direction, int pushIntervalTicks,
    int durationTicks) noexcept
    : _center(center), _radius(radius), _direction(direction),
      _pushIntervalTicks(pushIntervalTicks), _durationTicks(durationTicks)
{
}

/**
 * @brief Protocol name of the event.
 *
 * @return std::string_view "storm"
 */
std::string_view zappy::server::event::Storm::name() const noexcept
{
    return "storm";
}

/**
 * @brief Shared "<X> <Y> <R> <O>" payload for the broadcast lines.
 *
 * @return std::string the formatted zone arguments
 */
std::string zappy::server::event::Storm::zoneArguments() const
{
    return std::to_string(_center.x()) + " " + std::to_string(_center.y())
        + " " + std::to_string(_radius) + " "
        + std::to_string(static_cast<int>(_direction));
}

/**
 * @brief GUI line emitted when the storm starts.
 *
 * @return std::string "evt_storm_start <X> <Y> <R> <O>"
 */
std::string zappy::server::event::Storm::startBroadcast() const
{
    return "evt_storm_start " + zoneArguments();
}

/**
 * @brief GUI line emitted on each active tick.
 *
 * @return std::string "evt_storm_tick <X> <Y> <R> <O>"
 */
std::string zappy::server::event::Storm::tickBroadcast() const
{
    return "evt_storm_tick " + zoneArguments();
}

/**
 * @brief GUI line emitted when the storm ends.
 *
 * @return std::string "evt_storm_end"
 */
std::string zappy::server::event::Storm::endBroadcast() const
{
    return "evt_storm_end";
}

/**
 * @brief Advance the storm one tick, pushing players on push ticks.
 *
 * @param world the world to mutate
 * @return bool true while active, false once expired
 */
bool zappy::server::event::Storm::applyTick(game::World &world)
{
    _elapsedTicks++;
    if (_elapsedTicks <= _durationTicks
        && _elapsedTicks % _pushIntervalTicks == 0)
        pushPlayersInZone(world);
    return _elapsedTicks <= _durationTicks;
}

/**
 * @brief Wind the storm down; nothing to restore.
 *
 * @param world the world (unused)
 */
void zappy::server::event::Storm::onEnd(game::World &world)
{
    static_cast<void>(world);
}

/**
 * @brief Zone centre.
 *
 * @return game::Position the centre
 */
zappy::server::game::Position
zappy::server::event::Storm::center() const noexcept
{
    return _center;
}

/**
 * @brief Zone radius in tiles.
 *
 * @return int the radius
 */
int zappy::server::event::Storm::radius() const noexcept
{
    return _radius;
}

/**
 * @brief Push direction.
 *
 * @return game::Orientation the direction
 */
zappy::server::game::Orientation
zappy::server::event::Storm::direction() const noexcept
{
    return _direction;
}

/**
 * @brief Whether @p pos lies within the toroidal radius of the centre.
 *
 * @param pos position to test
 * @param worldWidth map width
 * @param worldHeight map height
 * @return bool true if squared distance <= radius squared
 */
bool zappy::server::event::Storm::isInZone(game::Position pos, int worldWidth,
    int worldHeight) const noexcept
{
    const game::Position delta =
        _center.shortestVectorTo(pos, worldWidth, worldHeight);
    const int distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();

    return distanceSquared <= _radius * _radius;
}

/**
 * @brief Push every caught, non-dead player one tile along the direction.
 *
 * @param world the world to mutate
 */
void zappy::server::event::Storm::pushPlayersInZone(game::World &world)
{
    const game::Position delta =
        game::OrientationHelper::forwardDelta(_direction);
    const int width = world.width();
    const int height = world.height();

    world.forEachPlayer(
        [this, &world, delta, width, height](int id,
            const game::Player &player) {
            if (player.state() != game::PlayerState::DEAD
                && isInZone(player.position(), width, height))
                world.movePlayer(id, (player.position() + delta)
                    .normalized(width, height));
        });
}
