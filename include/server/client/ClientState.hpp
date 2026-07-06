/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Lifecycle states a connected client can be in
*/

#ifndef CLIENT_CLIENTSTATE_HPP_
    #define CLIENT_CLIENTSTATE_HPP_

namespace zappy::server::client {

/**
 * @brief Lifecycle state of a connected client.
 *
 * Valid transitions: HANDSHAKE -> AI | GUI | GUI_ADMIN only.
 */
enum class ClientState {
    HANDSHAKE,
    AI,
    GUI,
    GUI_ADMIN
};

} // namespace zappy::server::client

#endif /* !CLIENT_CLIENTSTATE_HPP_ */
