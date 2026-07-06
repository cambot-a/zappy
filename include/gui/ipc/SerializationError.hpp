/*
** EPITECH PROJECT, 2026
** SerializationError.hpp
** File description:
** Typed exception thrown by the ipc functions parser on any communication error
*/

#ifndef CLIPARSEERROR_HPP_
    #define CLIPARSEERROR_HPP_
    #include <stdexcept>
    #include <string>

namespace zappy::gui::ipc {

/**
 * @brief Exception signalling a network communication error
 */
class SerializationError : public std::runtime_error {
public:
    /**
     * @brief Build an error with a reason.
     *
     * @param reason description of the invalid input
     */
    explicit SerializationError(const std::string &reason)
        : std::runtime_error(reason)
    {
    }
};

} // namespace zappy::gui::ipc

#endif /* CLIPARSEERROR_HPP_ */
