/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Tile implementation
*/

#include <algorithm>
#include <cstddef>

#include "server/game/Tile.hpp"

/**
 * @brief Quantity of @p type currently on the tile.
 *
 * @param type resource to query
 * @return int the stored quantity
 */
int zappy::server::game::Tile::resource(ResourceType type) const noexcept
{
    return _resources[static_cast<std::size_t>(type)];
}

/**
 * @brief Overwrite the quantity of @p type, never below zero.
 *
 * @param type resource to set
 * @param qty new quantity
 */
void zappy::server::game::Tile::setResource(ResourceType type, int qty) noexcept
{
    _resources[static_cast<std::size_t>(type)] = (qty < 0) ? 0 : qty;
}

/**
 * @brief Add @p qty units of @p type.
 *
 * @param type resource to add
 * @param qty quantity to add
 * @return bool false if qty is negative
 */
bool zappy::server::game::Tile::addResource(ResourceType type, int qty) noexcept
{
    const bool ok = qty >= 0;
    if (ok)
        _resources[static_cast<std::size_t>(type)] += qty;
    return ok;
}

/**
 * @brief Remove @p qty units of @p type.
 *
 * @param type resource to remove
 * @param qty quantity to remove
 * @return bool false if qty is negative or stock is insufficient
 */
bool zappy::server::game::Tile::removeResource(
    ResourceType type, int qty) noexcept
{
    const std::size_t index = static_cast<std::size_t>(type);
    const bool ok = qty >= 0 && _resources[index] >= qty;
    if (ok)
        _resources[index] -= qty;
    return ok;
}

/**
 * @brief Ids of players standing on the tile.
 *
 * @return const std::vector<int>& the player id list
 */
const std::vector<int> &
zappy::server::game::Tile::playerIds() const noexcept
{
    return _playerIds;
}

/**
 * @brief Ids of eggs lying on the tile.
 *
 * @return const std::vector<int>& the egg id list
 */
const std::vector<int> &
zappy::server::game::Tile::eggIds() const noexcept
{
    return _eggIds;
}

/**
 * @brief Register player @p id as present.
 *
 * @param id player id to add
 */
void zappy::server::game::Tile::addPlayer(int id)
{
    _playerIds.push_back(id);
}

/**
 * @brief Unregister player @p id; no-op if absent.
 *
 * @param id player id to remove
 */
void zappy::server::game::Tile::removePlayer(int id) noexcept
{
    std::erase(_playerIds, id);
}

/**
 * @brief Register egg @p id as present.
 *
 * @param id egg id to add
 */
void zappy::server::game::Tile::addEgg(int id)
{
    _eggIds.push_back(id);
}

/**
 * @brief Unregister egg @p id; no-op if absent.
 *
 * @param id egg id to remove
 */
void zappy::server::game::Tile::removeEgg(int id) noexcept
{
    std::erase(_eggIds, id);
}

/**
 * @brief Mark the tile as flooded or dry.
 *
 * @param value true to flood, false to dry out
 */
void zappy::server::game::Tile::setFlooded(bool value) noexcept
{
    _flooded = value;
}

/**
 * @brief Whether the tile is currently flooded.
 *
 * @return bool true if flooded
 */
bool zappy::server::game::Tile::isFlooded() const noexcept
{
    return _flooded;
}

/**
 * @brief Set the tile's terrain biome.
 *
 * @param biome the biome to assign
 */
void zappy::server::game::Tile::setBiome(Biome biome) noexcept
{
    _biome = biome;
}

/**
 * @brief The tile's terrain biome.
 *
 * @return Biome the current biome (PLAIN by default)
 */
zappy::server::game::Biome zappy::server::game::Tile::biome() const noexcept
{
    return _biome;
}
