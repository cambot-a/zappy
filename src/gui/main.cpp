/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** zappy_gui entry point
*/

#include <iostream>
#include <stdexcept>

#include "gui/Constants.hpp"
#include "gui/Gui.hpp"
#include "gui/ipc/SerializationError.hpp"
#include "gui/GuiArgParser.hpp"

/**
 * @brief zappy_gui entry point: server_infor then hand off to Gui.
 *
 * @param argc number of command-line arguments (valid = 2 if help otherwise 2)
 * @param argv command-line argument vector
 * @return int SUCCESS on clean shutdown, FAILURE on any error
 */
int main(int argc, char **argv)
{
    int exitCode = zappy::gui::SUCCESS;

    if (zappy::gui::GuiArgParser::wantsHelp(argc, argv)) {
        std::cout << zappy::gui::GuiArgParser::usage();
        return zappy::gui::SUCCESS;
    }
    try {
        zappy::gui::Gui(zappy::gui::GuiArgParser::parse(argc, argv)).run();
    } catch (const zappy::gui::ipc::SerializationError &error) {
        std::cerr << "Error: " << error.what() << "\n"
                  << zappy::gui::GuiArgParser::usage();
        exitCode = zappy::gui::FAILURE;
    } catch (const std::runtime_error &error) {
        std::cerr << "Error: " << error.what() << "\n";
        exitCode = zappy::gui::FAILURE;
    }
    return exitCode;
}
