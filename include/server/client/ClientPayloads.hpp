/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Per-state payload PODs stored inside Client
*/

#ifndef CLIENT_CLIENTPAYLOADS_HPP_
    #define CLIENT_CLIENTPAYLOADS_HPP_

    #include <string>
    #include <variant>

namespace zappy::server::client {

/**
 * @brief Payload for AI clients.
 */
struct AiData {
    std::string teamName;
    int playerId = 0;
};

/**
 * @brief Payload for GUI clients (no fields yet).
 */
struct GuiData {};

/**
 * @brief Payload for GUI_ADMIN clients (no fields yet).
 */
struct GuiAdminData {};

/**
 * @brief Closed-set variant of all possible client payloads.
 *        std::monostate is active during HANDSHAKE.
 */
typedef std::variant<std::monostate, AiData, GuiData, GuiAdminData> ClientPayload;

} // namespace zappy::server::client

#endif /* !CLIENT_CLIENTPAYLOADS_HPP_ */
