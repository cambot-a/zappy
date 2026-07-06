/*
** EPITECH PROJECT, 2026
** ServerInfo.hpp
** File description:
** Server configuration produced by the CLI parser
*/

#ifndef SERVERINFO_HPP_
    #define SERVERINFO_HPP_
    #include <cstdint>
    #include <string>
    #include <utility>
    #include <iostream>

namespace zappy::gui::ipc {

/**
 * @brief Immutable validated configuration consumed by the server at boot.
 */
class ServerInfo {
public:
    /**
     * @brief Construct a server address from validated values.
     *
     * @param port listening port (1..65535)
     * @param _hostname hostname of the server
     */
    ServerInfo(std::uint16_t server_port, std::string server_hostname)
        : _port(server_port), _server_hostname(std::move(server_hostname))
    {
    }

    /**
     * @brief Listening port number.
     *
     * @return std::uint16_t the port
     */
    std::uint16_t port() const noexcept { return _port; }

    /**
     * @brief Namespace of the server.
     *
     * @return std::string the namespace
     */
    std::string server_hostname() const noexcept { return _server_hostname; }

    friend std::ostream& operator<<(std::ostream& os,
                                    const ServerInfo& server)
    {
        return os << "ServerInfo { "
                  << "hostname: " << server._server_hostname
                  << ", port: " << server._port
                  << " }";
    }

private:
    const std::uint16_t _port;
    const std::string _server_hostname;
};

}

#endif /* SERVERINFO_HPP_ */
