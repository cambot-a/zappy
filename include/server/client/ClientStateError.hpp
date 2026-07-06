/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Exception for invalid state transitions and absent client lookups
*/

#ifndef CLIENT_CLIENTSTATEERROR_HPP_
    #define CLIENT_CLIENTSTATEERROR_HPP_

    #include <stdexcept>
    #include <string>

namespace zappy::server::client {

/**
 * @brief Thrown on invalid state transitions, wrong typed-accessor calls,
 *        or absent fd lookups in ClientRegistry.
 */
class ClientStateError : public std::runtime_error {
public:
    /**
     * @brief Build an error with a reason.
     *
     * @param reason description of the error
     */
    explicit ClientStateError(const std::string &reason)
        : std::runtime_error(reason)
    {}
};

} // namespace zappy::server::client

#endif /* !CLIENT_CLIENTSTATEERROR_HPP_ */
