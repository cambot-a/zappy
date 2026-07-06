/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Observer interface notified of every world mutation
*/

#ifndef SERVER_GAME_IWORLDOBSERVER_HPP_
    #define SERVER_GAME_IWORLDOBSERVER_HPP_

    #include <string>
    #include <vector>

    #include "server/game/Position.hpp"
    #include "server/game/Constants.hpp"
    #include "server/game/Biome.hpp"

namespace zappy::server::game {

/**
 * @brief Pure interface invoked by World on each state change.
 */
class IWorldObserver {
public:
    virtual ~IWorldObserver() = default;

    /**
     * @brief A tile's resources or occupants changed.
     *
     * @param pos the affected tile
     */
    virtual void onTileChanged(Position pos) = 0;

    /**
     * @brief A player was added to the world.
     *
     * @param id the new player id
     */
    virtual void onPlayerAdded(int id) = 0;

    /**
     * @brief A player moved between two tiles.
     *
     * @param id the player id
     * @param oldPos previous tile
     * @param newPos new tile
     */
    virtual void onPlayerMoved(int id, Position oldPos, Position newPos) = 0;

    /**
     * @brief A player changed orientation.
     *
     * @param id the player id
     */
    virtual void onPlayerRotated(int id) = 0;

    /**
     * @brief A player's level changed.
     *
     * @param id the player id
     */
    virtual void onPlayerLevelChanged(int id) = 0;

    /**
     * @brief A player's inventory changed.
     *
     * @param id the player id
     */
    virtual void onPlayerInventoryChanged(int id) = 0;

    /**
     * @brief A player's lifecycle state changed.
     *
     * @param id the player id
     */
    virtual void onPlayerStateChanged(int id) = 0;

    /**
     * @brief A player was removed from the world.
     *
     * @param id the player id
     */
    virtual void onPlayerRemoved(int id) = 0;

    /**
     * @brief An egg was added to the world.
     *
     * @param id the new egg id
     */
    virtual void onEggAdded(int id) = 0;

    /**
     * @brief An egg hatched.
     *
     * @param id the egg id
     */
    virtual void onEggHatched(int id) = 0;

    /**
     * @brief An egg was removed from the world.
     *
     * @param id the egg id
     */
    virtual void onEggRemoved(int id) = 0;

    /**
     * @brief A team's slot count changed.
     *
     * @param teamName the affected team
     */
    virtual void onTeamSlotsChanged(const std::string &teamName) = 0;

    /**
     * @brief A player broadcast a message to the world.
     *
     * @param id the broadcasting player id
     * @param text the broadcast text
     */
    virtual void onPlayerBroadcast(int id, const std::string &text) = 0;

    /**
     * @brief A player started a Fork command.
     *
     * @param playerId the forking player id
     */
    virtual void onPlayerForkStarted(int playerId) = 0;

    /**
     * @brief An incantation ritual started on a tile.
     *
     * @param initiatorId the player who launched the incantation
     * @param level the level being elevated from
     * @param participants ids of every frozen player taking part
     */
    virtual void onIncantationStarted(int initiatorId, int level,
        const std::vector<int> &participants) = 0;

    /**
     * @brief An incantation ritual ended.
     *
     * @param initiatorId the player who launched the incantation
     * @param success whether the elevation succeeded
     * @param newLevel the resulting level on success, 0 on failure
     */
    virtual void onIncantationEnded(int initiatorId, bool success,
        int newLevel) = 0;

    /**
     * @brief A player performed an Eject command.
     *
     * @param id player id
     */
    virtual void onPlayerEjected(int id) = 0;

    /**
     * @brief A player dropped a resource.
     *
     * @param id player id
     * @param type resource type
     */
    virtual void onPlayerDroppedResource(int id, ResourceType type) = 0;

    /**
     * @brief A player picked up a resource.
     *
     * @param id player id
     * @param type resource type
     */
    virtual void onPlayerPickedUpResource(int id, ResourceType type) = 0;

    /**
     * @brief A tile's flood state changed.
     *
     * @param pos the affected tile
     * @param flooded true if now flooded, false if dried out
     */
    virtual void onTileFloodChanged(Position pos, bool flooded) = 0;

    /**
     * @brief A tile's terrain biome changed.
     *
     * @param pos the affected tile
     * @param biome the new biome
     */
    virtual void onTileBiomeChanged(Position pos, Biome biome) = 0;
};

/**
 * @brief Empty default implementation; override only the events you need.
 */
class WorldObserverAdapter : public IWorldObserver {
public:
    void onTileChanged(Position) override {}
    void onPlayerAdded(int) override {}
    void onPlayerMoved(int, Position, Position) override {}
    void onPlayerRotated(int) override {}
    void onPlayerLevelChanged(int) override {}
    void onPlayerInventoryChanged(int) override {}
    void onPlayerStateChanged(int) override {}
    void onPlayerRemoved(int) override {}
    void onEggAdded(int) override {}
    void onEggHatched(int) override {}
    void onEggRemoved(int) override {}
    void onTeamSlotsChanged(const std::string &) override {}
    void onPlayerBroadcast(int, const std::string &) override {}
    void onPlayerForkStarted(int) override {}
    void onIncantationStarted(int, int, const std::vector<int> &) override {}
    void onIncantationEnded(int, bool, int) override {}
    void onPlayerEjected(int) override {}
    void onPlayerDroppedResource(int, ResourceType) override {}
    void onPlayerPickedUpResource(int, ResourceType) override {}
    void onTileFloodChanged(Position, bool) override {}
    void onTileBiomeChanged(Position, Biome) override {}
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_IWORLDOBSERVER_HPP_ */
