/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Project-wide gui constants
*/

#ifndef GUI_CONSTANTS_HPP_
    #define GUI_CONSTANTS_HPP_

    #include <cstddef>

namespace zappy::gui {

/// @brief Process exit status on a fatal error (subject requirement).
constexpr int FAILURE = 84;

/// @brief Process exit status on success.
constexpr int SUCCESS = 0;

/*
constexpr int INVALID_FD = -1;

/// @brief Size of the per-read client buffer.
constexpr std::size_t READ_BUFFER_SIZE = 1024;

/// @brief Chunk size for each ::read() call in ClientBuffer.
constexpr std::size_t READ_CHUNK_SIZE = 4096;

/// @brief Maximum bytes accumulated in the read buffer before dropping the client.
constexpr std::size_t MAX_READ_BUF = 1024U * 1024U;
*/
    
namespace {
    
constexpr int kDefaultPort = 4242;
constexpr const char *kDefaultNamespace = "localhost";

/*

/// @brief Require a positive value (> 0) for a flag, or throw.
void requirePositive(long value, std::string_view flag)
{
    if (value <= 0)
        throw ipc::SerializationError(std::string(flag) + " must be strictly positive");
}
*/

} // namespace

}

#endif /* !GUI_CONSTANTS_HPP_ */
