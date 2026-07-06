/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** zappy_server entry point
*/

#include <iostream>
#include <stdexcept>

#include "server/Constants.hpp"
#include "server/Server.hpp"
#include "server/cli/CliParseError.hpp"
#include "server/cli/CliParser.hpp"

/**
 * @brief zappy_server entry point: parse the CLI then hand off to Server.
 *
 * @param argc number of command-line arguments
 * @param argv command-line argument vector
 * @return int SUCCESS on clean shutdown, FAILURE on any error
 */
int main(int argc, char **argv)
{
    int exitCode = zappy::server::SUCCESS;

    try {
        const auto config = zappy::server::cli::CliParser{}.parse(argc, argv);
        if (config)
            zappy::server::Server(*config).run();
    } catch (const zappy::server::cli::CliParseError &error) {
        std::cerr << "Error: " << error.what() << "\n"
                  << zappy::server::cli::CliParser::usage();
        exitCode = zappy::server::FAILURE;
    } catch (const std::runtime_error &error) {
        std::cerr << "Error: " << error.what() << "\n";
        exitCode = zappy::server::FAILURE;
    }
    return exitCode;
}
