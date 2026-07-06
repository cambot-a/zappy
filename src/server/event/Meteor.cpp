/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Meteor event implementation
*/

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "server/event/Meteor.hpp"
#include "server/game/Player.hpp"
#include "server/game/World.hpp"

/**
 * @brief Build a meteor centred on @p center.
 *
 * @param center impact zone centre on the map
 * @param radius impact zone radius in tiles
 * @param onPlayerKilled callback closing a killed player's AI connection
 */
zappy::server::event::Meteor::Meteor(game::Position center, int radius,
    std::function<void(int playerId)> onPlayerKilled)
    : _center(center), _radius(radius),
      _onPlayerKilled(std::move(onPlayerKilled))
{
}

/**
 * @brief Protocol name of the event.
 *
 * @return std::string_view "meteor"
 */
std::string_view zappy::server::event::Meteor::name() const noexcept
{
    return "meteor";
}

/**
 * @brief GUI line emitted when the meteor strikes.
 *
 * @return std::string "evt_meteor_impact <X> <Y> <R>"
 */
std::string zappy::server::event::Meteor::startBroadcast() const
{
    return "evt_meteor_impact " + std::to_string(_center.x()) + " "
        + std::to_string(_center.y()) + " " + std::to_string(_radius);
}

/**
 * @brief GUI line emitted on each active tick.
 *
 * @return std::string the empty string (no per-tick broadcast)
 */
std::string zappy::server::event::Meteor::tickBroadcast() const
{
    return "";
}

/**
 * @brief GUI line emitted when the meteor ends.
 *
 * @return std::string "evt_meteor_end"
 */
std::string zappy::server::event::Meteor::endBroadcast() const
{
    return "evt_meteor_end";
}

/**
 * @brief Strike once: clear resources, kill players, then expire.
 *
 * @param world the world to mutate
 * @return bool always false (the meteor is a one-shot event)
 */
bool zappy::server::event::Meteor::applyTick(game::World &world)
{
    if (!_impacted) {
        _impacted = true;
        destroyResources(world);
        killPlayersInZone(world);
    }
    return false;
}

/**
 * @brief Wind the meteor down; nothing to restore.
 *
 * @param world the world (unused)
 */
void zappy::server::event::Meteor::onEnd(game::World &world)
{
    static_cast<void>(world);
}

/**
 * @brief Whether @p pos lies within the toroidal radius of the centre.
 *
 * @param pos position to test
 * @param worldWidth map width
 * @param worldHeight map height
 * @return bool true if squared distance <= radius squared
 */
bool zappy::server::event::Meteor::isInZone(game::Position pos, int worldWidth,
    int worldHeight) const noexcept
{
    const game::Position delta =
        _center.shortestVectorTo(pos, worldWidth, worldHeight);
    const int distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();

    return distanceSquared <= _radius * _radius;
}

/**
 * @brief Collect every tile position lying inside the impact zone.
 *
 * @param world the world supplying the map dimensions
 * @return std::vector<game::Position> the in-zone tile positions
 */
std::vector<zappy::server::game::Position>
zappy::server::event::Meteor::impactTiles(const game::World &world) const
{
    const int side = 2 * _radius + 1;
    std::vector<game::Position> tiles;

    for (int i = 0; i < side * side; i++) {
        const game::Position pos =
            game::Position(_center.x() + i % side - _radius,
                _center.y() + i / side - _radius)
                .normalized(world.width(), world.height());
        if (isInZone(pos, world.width(), world.height()))
            tiles.push_back(pos);
    }
    return tiles;
}

/**
 * @brief Zero every resource on each tile of the impact zone.
 *
 * @param world the world to mutate
 */
void zappy::server::event::Meteor::destroyResources(game::World &world)
{
    const std::vector<game::Position> tiles = impactTiles(world);

    std::ranges::for_each(tiles,
        [this, &world](game::Position pos) { clearTile(world, pos); });
}

/**
 * @brief Zero every resource on a single tile.
 *
 * @param world the world to mutate
 * @param pos the tile to clear
 */
void zappy::server::event::Meteor::clearTile(game::World &world,
    game::Position pos)
{
    for (std::size_t i = 0; i < game::RESOURCE_COUNT; i++)
        world.setTileResource(pos, static_cast<game::ResourceType>(i), 0);
}

/**
 * @brief Kill every alive player standing in the impact zone.
 *
 * @param world the world to mutate
 */
void zappy::server::event::Meteor::killPlayersInZone(game::World &world)
{
    const int width = world.width();
    const int height = world.height();
    std::vector<int> toKill;

    world.forEachPlayer([&](int id, const game::Player &player) {
        if (player.state() == game::PlayerState::ALIVE
            && isInZone(player.position(), width, height))
            toKill.push_back(id);
    });
    for (int id : toKill) {
        world.killPlayer(id);
        if (_onPlayerKilled)
            _onPlayerKilled(id);
    }
}
