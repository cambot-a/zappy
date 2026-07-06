/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** A team: a name and its total slot capacity
*/

#ifndef SERVER_GAME_TEAM_HPP_
    #define SERVER_GAME_TEAM_HPP_

    #include <string>

namespace zappy::server::game {

/**
 * @brief One team: a name and its total slot capacity.
 */
class Team {
public:
    /**
     * @brief Build a team with @p slotsTotal initial capacity.
     *
     * @param name team name
     * @param slotsTotal initial slot capacity
     */
    Team(std::string name, int slotsTotal);

    /**
     * @brief Team name.
     *
     * @return const std::string& the name
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Total slot capacity.
     *
     * @return int the total slots
     */
    [[nodiscard]] int slotsTotal() const noexcept;

    /**
     * @brief Grow the capacity by one slot (used by Fork).
     */
    void addSlot() noexcept;

private:
    std::string _name;
    int _slotsTotal;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_TEAM_HPP_ */
