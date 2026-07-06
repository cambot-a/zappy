/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** An egg laid on the map, awaiting a connecting AI client
*/

#ifndef SERVER_GAME_EGG_HPP_
    #define SERVER_GAME_EGG_HPP_

    #include <string>

    #include "server/game/Constants.hpp"
    #include "server/game/Position.hpp"

namespace zappy::server::game {

/**
 * @brief One egg: identity, owning team, position and hatch state.
 */
class Egg {
public:
    /**
     * @brief Build a fresh egg in the WAITING state.
     *
     * @param id unique egg id
     * @param team owning team name
     * @param position tile the egg lies on
     * @param layingPlayerId ID of the player who laid the egg
     */
    Egg(int id, std::string team, Position position, int layingPlayerId = -1);

    /**
     * @brief Unique egg id.
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
     * @brief Tile the egg lies on.
     *
     * @return Position the position
     */
    [[nodiscard]] Position position() const noexcept;

    /**
     * @brief Current hatch state.
     *
     * @return EggState the state
     */
    [[nodiscard]] EggState state() const noexcept;

    /**
     * @brief Set the egg's hatch state.
     *
     * @param state new state
     */
    void setState(EggState state) noexcept;

    /**
     * @brief Get the ID of the player who laid the egg.
     *
     * @return int player ID, or -1 if system/initial egg
     */
    [[nodiscard]] int layingPlayerId() const noexcept;

private:
    int _id;
    std::string _team;
    Position _position;
    EggState _state;
    int _layingPlayerId;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_EGG_HPP_ */
