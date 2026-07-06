/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Meteor natural event: one-shot impact destroying resources and players
*/

#ifndef SERVER_EVENT_METEOR_HPP_
    #define SERVER_EVENT_METEOR_HPP_

    #include <functional>
    #include <string>
    #include <string_view>
    #include <vector>

    #include "server/event/Event.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {
class World;
} // namespace zappy::server::game

namespace zappy::server::event {

/**
 * @brief Instant meteor strike clearing a circular impact zone.
 *
 * On its single tick the meteor zeroes every resource and kills every alive
 * player inside the toroidal radius of its centre, then expires at once. It
 * broadcasts evt_meteor_impact on start and evt_meteor_end immediately after;
 * there is no per-tick line and nothing to restore on end.
 */
class Meteor : public Event {
public:
    /**
     * @brief Build a meteor centred on @p center.
     *
     * @param center impact zone centre on the map
     * @param radius impact zone radius in tiles
     * @param onPlayerKilled callback closing a killed player's AI connection
     */
    Meteor(game::Position center, int radius,
        std::function<void(int playerId)> onPlayerKilled);

    /**
     * @brief Protocol name of the event.
     *
     * @return std::string_view "meteor"
     */
    [[nodiscard]] std::string_view name() const noexcept override;

    /**
     * @brief GUI line emitted when the meteor strikes.
     *
     * @return std::string "evt_meteor_impact <X> <Y> <R>"
     */
    [[nodiscard]] std::string startBroadcast() const override;

    /**
     * @brief GUI line emitted on each active tick.
     *
     * @return std::string the empty string (no per-tick broadcast)
     */
    [[nodiscard]] std::string tickBroadcast() const override;

    /**
     * @brief GUI line emitted when the meteor ends.
     *
     * @return std::string "evt_meteor_end"
     */
    [[nodiscard]] std::string endBroadcast() const override;

    /**
     * @brief Strike once: clear resources, kill players, then expire.
     *
     * @param world the world to mutate
     * @return bool always false (the meteor is a one-shot event)
     */
    [[nodiscard]] bool applyTick(game::World &world) override;

    /**
     * @brief Wind the meteor down; nothing to restore.
     *
     * @param world the world (unused)
     */
    void onEnd(game::World &world) override;

private:
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
     * @brief Collect every tile position lying inside the impact zone.
     *
     * @param world the world supplying the map dimensions
     * @return std::vector<game::Position> the in-zone tile positions
     */
    [[nodiscard]] std::vector<game::Position> impactTiles(
        const game::World &world) const;

    /**
     * @brief Zero every resource on each tile of the impact zone.
     *
     * @param world the world to mutate
     */
    void destroyResources(game::World &world);

    /**
     * @brief Zero every resource on a single tile.
     *
     * @param world the world to mutate
     * @param pos the tile to clear
     */
    void clearTile(game::World &world, game::Position pos);

    /**
     * @brief Kill every alive player standing in the impact zone.
     *
     * @param world the world to mutate
     */
    void killPlayersInZone(game::World &world);

    game::Position _center;
    int _radius;
    std::function<void(int playerId)> _onPlayerKilled;
    bool _impacted = false;
};

} // namespace zappy::server::event

#endif /* !SERVER_EVENT_METEOR_HPP_ */
