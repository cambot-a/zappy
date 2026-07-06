/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Project-wide server constants
*/

#ifndef SERVER_CONSTANTS_HPP_
    #define SERVER_CONSTANTS_HPP_

    #include <cstddef>

namespace zappy::server {

/// @brief Process exit status on a fatal error (subject requirement).
constexpr int FAILURE = 84;

/// @brief Process exit status on success.
constexpr int SUCCESS = 0;

constexpr int INVALID_FD = -1;

/// @brief Size of the per-read client buffer.
constexpr std::size_t READ_BUFFER_SIZE = 1024;

/// @brief Chunk size for each ::read() call in ClientBuffer.
constexpr std::size_t READ_CHUNK_SIZE = 4096;

/// @brief Maximum bytes accumulated in the read buffer before dropping the client.
constexpr std::size_t MAX_READ_BUF = 1024U * 1024U;

}

#endif /* !SERVER_CONSTANTS_HPP_ */
