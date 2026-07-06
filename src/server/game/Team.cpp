/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Team implementation
*/

#include <utility>

#include "server/game/Team.hpp"

/**
 * @brief Build a team with @p slotsTotal initial capacity.
 *
 * @param name team name
 * @param slotsTotal initial slot capacity
 */
zappy::server::game::Team::Team(std::string name, int slotsTotal)
    : _name(std::move(name)), _slotsTotal(slotsTotal)
{}

/**
 * @brief Team name.
 *
 * @return const std::string& the name
 */
const std::string &zappy::server::game::Team::name() const noexcept
{
    return _name;
}

/**
 * @brief Total slot capacity.
 *
 * @return int the total slots
 */
int zappy::server::game::Team::slotsTotal() const noexcept
{
    return _slotsTotal;
}

/**
 * @brief Grow the capacity by one slot (used by Fork).
 */
void zappy::server::game::Team::addSlot() noexcept
{
    ++_slotsTotal;
}
