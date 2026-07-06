/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Central authoritative game state
*/

#ifndef SERVER_GAME_WORLD_HPP_
    #define SERVER_GAME_WORLD_HPP_

    #include <cstddef>
    #include <functional>
    #include <optional>
    #include <random>
    #include <string>
    #include <unordered_map>
    #include <vector>

    #include "server/game/Constants.hpp"
    #include "server/game/Egg.hpp"
    #include "server/game/IWorldObserver.hpp"
    #include "server/game/Player.hpp"
    #include "server/game/Position.hpp"
    #include "server/game/Team.hpp"
    #include "server/game/Tile.hpp"

namespace zappy::server::game {

/**
 * @brief Owns the grid, teams, players and eggs; notifies observers on change.
 */
class World {
public:
    /**
     * @brief Build an empty world: grid of tiles plus the configured teams.
     *
     * @param width map width in tiles
     * @param height map height in tiles
     * @param teamNames configured team names
     * @param slotsPerTeam slot capacity per team
     */
    World(int width, int height,
        const std::vector<std::string> &teamNames, int slotsPerTeam);

    World(const World &) = delete;
    World &operator=(const World &) = delete;
    World(World &&) = default;
    World &operator=(World &&) = default;

    /**
     * @brief Map width in tiles.
     *
     * @return int the width
     */
    [[nodiscard]] int width() const noexcept;

    /**
     * @brief Map height in tiles.
     *
     * @return int the height
     */
    [[nodiscard]] int height() const noexcept;

    /**
     * @brief Tile at an already-normalized position.
     *
     * @param pos normalized position
     * @return Tile& the tile
     */
    [[nodiscard]] Tile &tileAt(Position pos) noexcept;

    /**
     * @brief Tile at an already-normalized position.
     *
     * @param pos normalized position
     * @return const Tile& the tile
     */
    [[nodiscard]] const Tile &tileAt(Position pos) const noexcept;

    /**
     * @brief Tile at any position, normalizing first.
     *
     * @param pos possibly out-of-range position
     * @return Tile& the tile
     */
    [[nodiscard]] Tile &tileAtRaw(Position pos) noexcept;

    /**
     * @brief Tile at any position, normalizing first.
     *
     * @param pos possibly out-of-range position
     * @return const Tile& the tile
     */
    [[nodiscard]] const Tile &tileAtRaw(Position pos) const noexcept;

    /**
     * @brief Whether @p name is a known team.
     *
     * @param name team name
     * @return bool true if known
     */
    [[nodiscard]] bool hasTeam(const std::string &name) const noexcept;

    /**
     * @brief Mutable team by name.
     *
     * @param name team name
     * @return Team& the team
     * @throws WorldError if unknown
     */
    [[nodiscard]] Team &team(const std::string &name);

    /**
     * @brief Read-only team by name.
     *
     * @param name team name
     * @return const Team& the team
     * @throws WorldError if unknown
     */
    [[nodiscard]] const Team &team(const std::string &name) const;

    /**
     * @brief All teams in configuration order.
     *
     * @return const std::vector<Team>& the teams
     */
    [[nodiscard]] const std::vector<Team> &teams() const noexcept;

    /**
     * @brief Whether a player with @p id exists.
     *
     * @param id player id
     * @return bool true if present
     */
    [[nodiscard]] bool hasPlayer(int id) const noexcept;

    /**
     * @brief Mutable player by id.
     *
     * @param id player id
     * @return Player& the player
     * @throws WorldError if unknown
     */
    [[nodiscard]] Player &player(int id);

    /**
     * @brief Read-only player by id.
     *
     * @param id player id
     * @return const Player& the player
     * @throws WorldError if unknown
     */
    [[nodiscard]] const Player &player(int id) const;

    /**
     * @brief Invoke @p callback for every (id, Player const &) in the world.
     *
     * @param callback callable receiving the player id and a const reference
     */
    template<typename F>
    void forEachPlayer(F &&callback) const
    {
        for (const auto &entry : _players)
            callback(entry.first, entry.second);
    }

    /**
     * @brief Invoke @p callback for every (id, Egg const &) in the world.
     *
     * @param callback callable receiving the egg id and a const reference
     */
    template<typename F>
    void forEachEgg(F &&callback) const
    {
        for (const auto &entry : _eggs)
            callback(entry.first, entry.second);
    }

    /**
     * @brief Create a player and place it on its tile.
     *
     * @param team owning team name
     * @param pos starting tile
     * @param orient starting facing
     * @return int the new player id
     * @throws WorldError if the team is unknown
     */
    int addPlayer(const std::string &team, Position pos, Orientation orient);

    /**
     * @brief Remove a player from the world and its tile.
     *
     * @param id player id
     * @throws WorldError if unknown
     */
    void removePlayer(int id);

    /**
     * @brief Whether an egg with @p id exists.
     *
     * @param id egg id
     * @return bool true if present
     */
    [[nodiscard]] bool hasEgg(int id) const noexcept;

    /**
     * @brief Mutable egg by id.
     *
     * @param id egg id
     * @return Egg& the egg
     * @throws WorldError if unknown
     */
    [[nodiscard]] Egg &egg(int id);

    /**
     * @brief Read-only egg by id.
     *
     * @param id egg id
     * @return const Egg& the egg
     * @throws WorldError if unknown
     */
    [[nodiscard]] const Egg &egg(int id) const;

    /**
     * @brief Create an egg and place it on its tile.
     *
     * @param team owning team name
     * @param pos tile to lay the egg on
     * @param layingPlayerId ID of the player laying the egg
     * @return int the new egg id
     */
    int addEgg(const std::string &team, Position pos, int layingPlayerId = -1);

    /**
     * @brief Remove an egg from the world and its tile.
     *
     * @param id egg id
     * @throws WorldError if unknown
     */
    void removeEgg(int id);

    /**
     * @brief Count WAITING eggs owned by @p teamName.
     *
     * @param teamName team to count for; unknown teams yield 0
     * @return int the number of waiting eggs
     */
    [[nodiscard]] int waitingEggCount(
        const std::string &teamName) const noexcept;

    /**
     * @brief Pick one WAITING egg of @p teamName uniformly at random.
     *
     * @param teamName team whose eggs are eligible
     * @param rng random engine driving the choice
     * @return std::optional<int> the chosen egg id, or nullopt if none
     */
    [[nodiscard]] std::optional<int> pickRandomWaitingEgg(
        const std::string &teamName, std::mt19937_64 &rng) const;

    /**
     * @brief Grow a team's capacity by one slot and notify observers.
     *
     * @param teamName team to grow
     * @throws WorldError if the team is unknown
     */
    void growTeam(const std::string &teamName);

    /**
     * @brief Move a player to @p newPos (assumed normalized).
     *
     * @param playerId player id
     * @param newPos destination tile
     * @throws WorldError if unknown
     */
    void movePlayer(int playerId, Position newPos);

    /**
     * @brief Rotate a player to @p newOrient.
     *
     * @param playerId player id
     * @param newOrient new facing
     * @throws WorldError if unknown
     */
    void rotatePlayer(int playerId, Orientation newOrient);

    /**
     * @brief Set a player's elevation level.
     *
     * @param playerId player id
     * @param newLevel new level
     * @throws WorldError if unknown
     */
    void setPlayerLevel(int playerId, int newLevel);

    /**
     * @brief Move one unit of @p type from the player's tile to its inventory.
     *
     * @param playerId player id
     * @param type resource to take
     * @return bool false if the tile has none
     * @throws WorldError if unknown
     */
    bool takeResourceFromTile(int playerId, ResourceType type);

    /**
     * @brief Move one unit of @p type from the player's inventory to its tile.
     *
     * @param playerId player id
     * @param type resource to drop
     * @return bool false if the player has none
     * @throws WorldError if unknown
     */
    bool dropResourceOnTile(int playerId, ResourceType type);

    /**
     * @brief Kill a player: mark DEAD and remove it from its tile.
     *
     * @param playerId player id
     * @throws WorldError if unknown
     */
    void killPlayer(int playerId);

    /**
     * @brief Consume one unit of food from player's inventory and notify.
     *
     * @param playerId player id
     * @throws WorldError if unknown
     */
    void consumePlayerFood(int playerId);

    /**
     * @brief Notify observers a player ejected others.
     *
     * @param playerId player id
     */
    void notifyPlayerEjected(int playerId);

    /**
     * @brief Freeze a player for an incantation (state INCANTING).
     *
     * @param playerId player id
     * @throws WorldError if unknown
     */
    void freezePlayer(int playerId);

    /**
     * @brief Unfreeze a player back to ALIVE after an incantation.
     *
     * @param playerId player id
     * @throws WorldError if unknown
     */
    void unfreezePlayer(int playerId);

    /**
     * @brief Hatch an egg (state HATCHED); the egg is not removed here.
     *
     * @param eggId egg id
     * @throws WorldError if unknown
     */
    void hatchEgg(int eggId);

    /**
     * @brief Overwrite a tile's resource quantity.
     *
     * @param pos tile position
     * @param type resource to set
     * @param qty new quantity
     */
    void setTileResource(Position pos, ResourceType type, int qty);

    /**
     * @brief Add to a tile's resource quantity.
     *
     * @param pos tile position
     * @param type resource to add
     * @param qty quantity to add
     */
    void addTileResource(Position pos, ResourceType type, int qty);

    /**
     * @brief Set a tile's runtime flood state and notify observers.
     *
     * @param pos tile position
     * @param value true to flood, false to dry out
     */
    void setTileFlooded(Position pos, bool value);

    /**
     * @brief Set a tile's terrain biome and notify observers.
     *
     * @param pos tile position
     * @param biome the biome to assign
     */
    void setTileBiome(Position pos, Biome biome);

    /**
     * @brief Register an observer for subsequent mutations.
     *
     * @param observer observer to add
     */
    void addObserver(IWorldObserver &observer);

    /**
     * @brief Unregister an observer; no-op if not registered.
     *
     * @param observer observer to remove
     */
    void removeObserver(IWorldObserver &observer);

    /**
     * @brief Notify observers a player broadcast a message.
     *
     * @param id the broadcasting player id
     * @param text the broadcast text
     */
    void notifyBroadcast(int id, const std::string &text);

    /**
     * @brief Notify observers a player started a Fork command.
     *
     * @param playerId the forking player id
     */
    void notifyForkStarted(int playerId);

    /**
     * @brief Notify observers an incantation ritual started.
     *
     * @param initiatorId the player who launched the incantation
     * @param level the level being elevated from
     * @param participants ids of every frozen player taking part
     */
    void notifyIncantationStarted(int initiatorId, int level,
        const std::vector<int> &participants);

    /**
     * @brief Notify observers an incantation ritual ended.
     *
     * @param initiatorId the player who launched the incantation
     * @param success whether the elevation succeeded
     * @param newLevel the resulting level on success, 0 on failure
     */
    void notifyIncantationEnded(int initiatorId, bool success, int newLevel);

private:
    /**
     * @brief Row-major grid index of a normalized position.
     *
     * @param pos normalized position
     * @return std::size_t the index into the grid
     */
    [[nodiscard]] std::size_t tileIndex(Position pos) const noexcept;

    void notifyTileChanged(Position pos);
    void notifyPlayerAdded(int id);
    void notifyPlayerMoved(int id, Position oldPos, Position newPos);
    void notifyPlayerRotated(int id);
    void notifyPlayerLevelChanged(int id);
    void notifyPlayerInventoryChanged(int id);
    void notifyPlayerDroppedResource(int playerId, ResourceType type);
    void notifyPlayerPickedUpResource(int playerId, ResourceType type);
    void notifyTileFloodChanged(Position pos, bool flooded);
    void notifyTileBiomeChanged(Position pos, Biome biome);
    void notifyPlayerStateChanged(int id);
    void notifyPlayerRemoved(int id);
    void notifyEggAdded(int id);
    void notifyEggHatched(int id);
    void notifyEggRemoved(int id);
    void notifyTeamSlotsChanged(const std::string &teamName);

    int _width;
    int _height;
    std::vector<Tile> _grid;
    std::vector<Team> _teams;
    std::unordered_map<int, Player> _players;
    std::unordered_map<int, Egg> _eggs;
    int _nextPlayerId;
    int _nextEggId;
    std::vector<std::reference_wrapper<IWorldObserver>> _observers;
    std::vector<int> _activeEffects;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_WORLD_HPP_ */
