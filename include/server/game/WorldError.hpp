/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Exception thrown on world invariant violations
*/

#ifndef SERVER_GAME_WORLDERROR_HPP_
    #define SERVER_GAME_WORLDERROR_HPP_

    #include <stdexcept>
    #include <string>

namespace zappy::server::game {

/**
 * @brief Thrown on unknown player/team/egg ids or on a full team reservation.
 */
class WorldError : public std::runtime_error {
public:
    /**
     * @brief Build an error with a descriptive reason.
     *
     * @param reason human-readable description
     */
    explicit WorldError(const std::string &reason)
        : std::runtime_error(reason) {}
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_WORLDERROR_HPP_ */
