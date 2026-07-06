/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Typed exception for network layer failures
*/

#ifndef NET_NETWORKERROR_HPP_
    #define NET_NETWORKERROR_HPP_

    #include <stdexcept>
    #include <string>

namespace zappy::net {

/**
 * @brief Exception signalling a network layer failure.
 */
class NetworkError : public std::runtime_error {
public:
    /**
     * @brief Build an error with a reason.
     *
     * @param reason description of the failure
     */
    explicit NetworkError(const std::string &reason)
        : std::runtime_error(reason)
    {
    }
};

} // namespace zappy::net

#endif /* !NET_NETWORKERROR_HPP_ */
