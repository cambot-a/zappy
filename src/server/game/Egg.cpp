/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Egg implementation
*/

#include <utility>

#include "server/game/Egg.hpp"

/**
 * @brief Build a fresh egg in the WAITING state.
 *
 * @param id unique egg id
 * @param team owning team name
 * @param position tile the egg lies on
 */
zappy::server::game::Egg::Egg(int id, std::string team, Position position,
    int layingPlayerId)
    : _id(id), _team(std::move(team)), _position(position),
        _state(EggState::WAITING), _layingPlayerId(layingPlayerId)
{}

/**
 * @brief Unique egg id.
 *
 * @return int the id
 */
int zappy::server::game::Egg::id() const noexcept
{
    return _id;
}

/**
 * @brief Owning team name.
 *
 * @return const std::string& the team name
 */
const std::string &zappy::server::game::Egg::team() const noexcept
{
    return _team;
}

/**
 * @brief Tile the egg lies on.
 *
 * @return Position the position
 */
zappy::server::game::Position
zappy::server::game::Egg::position() const noexcept
{
    return _position;
}

/**
 * @brief Current hatch state.
 *
 * @return EggState the state
 */
zappy::server::game::EggState
zappy::server::game::Egg::state() const noexcept
{
    return _state;
}

/**
 * @brief Set the egg's hatch state.
 *
 * @param state new state
 */
void zappy::server::game::Egg::setState(EggState state) noexcept
{
    _state = state;
}

/**
 * @brief Get the ID of the player who laid this egg.
 *
 * @return int player ID, or -1 if system/initial egg
 */
int zappy::server::game::Egg::layingPlayerId() const noexcept
{
    return _layingPlayerId;
}
