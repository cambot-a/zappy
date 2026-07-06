/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Player implementation
*/

#include <cstddef>
#include <utility>

#include "server/game/Player.hpp"

/**
 * @brief Build a fresh player at level 1 with 10 food, state ALIVE.
 *
 * @param id unique player id
 * @param team owning team name
 * @param position starting tile
 * @param orientation starting facing
 */
zappy::server::game::Player::Player(int id, std::string team,
    Position position, Orientation orientation)
    : _id(id), _team(std::move(team)), _position(position),
        _orientation(orientation), _level(1), _state(PlayerState::ALIVE)
{
    _inventory[static_cast<std::size_t>(ResourceType::FOOD)] = INITIAL_FOOD;
}

/**
 * @brief Unique player id.
 *
 * @return int the id
 */
int zappy::server::game::Player::id() const noexcept
{
    return _id;
}

/**
 * @brief Owning team name.
 *
 * @return const std::string& the team name
 */
const std::string &zappy::server::game::Player::team() const noexcept
{
    return _team;
}

/**
 * @brief Current tile position.
 *
 * @return Position the position
 */
zappy::server::game::Position
zappy::server::game::Player::position() const noexcept
{
    return _position;
}

/**
 * @brief Current facing.
 *
 * @return Orientation the orientation
 */
zappy::server::game::Orientation
zappy::server::game::Player::orientation() const noexcept
{
    return _orientation;
}

/**
 * @brief Current elevation level.
 *
 * @return int the level
 */
int zappy::server::game::Player::level() const noexcept
{
    return _level;
}

/**
 * @brief Read-only inventory.
 *
 * @return const Inventory& the inventory
 */
const zappy::server::game::Inventory &
zappy::server::game::Player::inventory() const noexcept
{
    return _inventory;
}

/**
 * @brief Current lifecycle state.
 *
 * @return PlayerState the state
 */
zappy::server::game::PlayerState
zappy::server::game::Player::state() const noexcept
{
    return _state;
}

/**
 * @brief Move the player to @p position.
 *
 * @param position new tile position
 */
void zappy::server::game::Player::setPosition(Position position) noexcept
{
    _position = position;
}

/**
 * @brief Set the player's facing.
 *
 * @param orientation new orientation
 */
void zappy::server::game::Player::setOrientation(
    Orientation orientation) noexcept
{
    _orientation = orientation;
}

/**
 * @brief Set the player's elevation level.
 *
 * @param level new level
 */
void zappy::server::game::Player::setLevel(int level) noexcept
{
    _level = level;
}

/**
 * @brief Set the player's lifecycle state.
 *
 * @param state new state
 */
void zappy::server::game::Player::setState(PlayerState state) noexcept
{
    _state = state;
}

/**
 * @brief Quantity of @p type held by the player.
 *
 * @param type resource to query
 * @return int the held quantity
 */
int zappy::server::game::Player::resource(ResourceType type) const noexcept
{
    return _inventory[static_cast<std::size_t>(type)];
}

/**
 * @brief Add @p qty units of @p type to the inventory.
 *
 * @param type resource to add
 * @param qty quantity to add
 * @return bool false if qty is negative
 */
bool zappy::server::game::Player::addResource(
    ResourceType type, int qty) noexcept
{
    const bool ok = qty >= 0;
    if (ok)
        _inventory[static_cast<std::size_t>(type)] += qty;
    return ok;
}

/**
 * @brief Remove @p qty units of @p type from the inventory.
 *
 * @param type resource to remove
 * @param qty quantity to remove
 * @return bool false if qty is negative or stock is insufficient
 */
bool zappy::server::game::Player::removeResource(
    ResourceType type, int qty) noexcept
{
    const std::size_t index = static_cast<std::size_t>(type);
    const bool ok = qty >= 0 && _inventory[index] >= qty;
    if (ok)
        _inventory[index] -= qty;
    return ok;
}
