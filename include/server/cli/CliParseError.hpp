/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Typed exception thrown by the CLI parser on any invalid input
*/

#ifndef CLI_CLIPARSEERROR_HPP_
    #define CLI_CLIPARSEERROR_HPP_

    #include <stdexcept>
    #include <string>

namespace zappy::server::cli {

/**
 * @brief Exception signalling an invalid command line.
 */
class CliParseError : public std::runtime_error {
public:
    /**
     * @brief Build an error with a reason.
     *
     * @param reason description of the invalid input
     */
    explicit CliParseError(const std::string &reason)
        : std::runtime_error(reason)
    {
    }
};

}

#endif /* !CLI_CLIPARSEERROR_HPP_ */
