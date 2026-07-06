/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** A connected AI player on the map
*/

#ifndef SERVER_GAME_PLAYER_HPP_
    #define SERVER_GAME_PLAYER_HPP_

    #include <string>

    #include "server/game/Constants.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {

/**
 * @brief One AI participant: identity, pose, level and inventory.
 */
class Player {
public:
    /**
     * @brief Build a fresh player at level 1 with 10 food, state ALIVE.
     *
     * @param id unique player id
     * @param team owning team name
     * @param position starting tile
     * @param orientation starting facing
     */
    Player(int id, std::string team, Position position,
        Orientation orientation);

    /**
     * @brief Unique player id.
     *
     * @return int the id
     */
    [[nodiscard]] int id() const noexcept;

    /**
     * @brief Owning team name.
     *
     * @return const std::string& the team name
     */
    [[nodiscard]] const std::string &team() const noexcept;

    /**
     * @brief Current tile position.
     *
     * @return Position the position
     */
    [[nodiscard]] Position position() const noexcept;

    /**
     * @brief Current facing.
     *
     * @return Orientation the orientation
     */
    [[nodiscard]] Orientation orientation() const noexcept;

    /**
     * @brief Current elevation level.
     *
     * @return int the level
     */
    [[nodiscard]] int level() const noexcept;

    /**
     * @brief Read-only inventory.
     *
     * @return const Inventory& the inventory
     */
    [[nodiscard]] const Inventory &inventory() const noexcept;

    /**
     * @brief Current lifecycle state.
     *
     * @return PlayerState the state
     */
    [[nodiscard]] PlayerState state() const noexcept;

    /**
     * @brief Move the player to @p position.
     *
     * @param position new tile position
     */
    void setPosition(Position position) noexcept;

    /**
     * @brief Set the player's facing.
     *
     * @param orientation new orientation
     */
    void setOrientation(Orientation orientation) noexcept;

    /**
     * @brief Set the player's elevation level.
     *
     * @param level new level
     */
    void setLevel(int level) noexcept;

    /**
     * @brief Set the player's lifecycle state.
     *
     * @param state new state
     */
    void setState(PlayerState state) noexcept;

    /**
     * @brief Quantity of @p type held by the player.
     *
     * @param type resource to query
     * @return int the held quantity
     */
    [[nodiscard]] int resource(ResourceType type) const noexcept;

    /**
     * @brief Add @p qty units of @p type to the inventory.
     *
     * @param type resource to add
     * @param qty quantity to add (must be >= 0)
     * @return bool false if qty is negative
     */
    bool addResource(ResourceType type, int qty) noexcept;

    /**
     * @brief Remove @p qty units of @p type from the inventory.
     *
     * @param type resource to remove
     * @param qty quantity to remove (must be >= 0)
     * @return bool false if qty is negative or stock is insufficient
     */
    bool removeResource(ResourceType type, int qty) noexcept;

private:
    int _id;
    std::string _team;
    Position _position;
    Orientation _orientation;
    int _level;
    Inventory _inventory{};
    PlayerState _state;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_PLAYER_HPP_ */
