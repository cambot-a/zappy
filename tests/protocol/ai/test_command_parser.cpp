/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the AI command parser
*/

#include <criterion/criterion.h>
#include <string>

#include "protocol/ai/CommandKind.hpp"
#include "protocol/ai/CommandParser.hpp"

using zappy::protocol::ai::CommandKind;
using zappy::protocol::ai::CommandParser;
using zappy::protocol::ai::ParsedCommand;

/* every command name parses to its kind */

Test(command_parser, all_argumentless_commands_parse)
{
    cr_assert(CommandParser::parse("Forward")->kind == CommandKind::FORWARD);
    cr_assert(CommandParser::parse("Right")->kind == CommandKind::RIGHT);
    cr_assert(CommandParser::parse("Left")->kind == CommandKind::LEFT);
    cr_assert(CommandParser::parse("Look")->kind == CommandKind::LOOK);
    cr_assert(CommandParser::parse("Inventory")->kind
        == CommandKind::INVENTORY);
    cr_assert(CommandParser::parse("Connect_nbr")->kind
        == CommandKind::CONNECT_NBR);
    cr_assert(CommandParser::parse("Fork")->kind == CommandKind::FORK);
    cr_assert(CommandParser::parse("Eject")->kind == CommandKind::EJECT);
    cr_assert(CommandParser::parse("Incantation")->kind
        == CommandKind::INCANTATION);
}

Test(command_parser, argument_commands_parse_with_argument)
{
    cr_assert(CommandParser::parse("Broadcast hi")->kind
        == CommandKind::BROADCAST);
    cr_assert(CommandParser::parse("Take food")->kind == CommandKind::TAKE);
    cr_assert(CommandParser::parse("Set linemate")->kind == CommandKind::SET);
}

/* case sensitivity */

Test(command_parser, lowercase_name_is_rejected)
{
    cr_assert_not(CommandParser::parse("forward").has_value());
    cr_assert(CommandParser::parse("Forward").has_value());
}

/* argument extraction */

Test(command_parser, take_extracts_single_argument)
{
    const auto cmd = CommandParser::parse("Take food");
    cr_assert(cmd.has_value());
    cr_assert(cmd->kind == CommandKind::TAKE);
    cr_assert_str_eq(cmd->argument.c_str(), "food");
}

Test(command_parser, broadcast_keeps_multi_word_argument)
{
    const auto cmd = CommandParser::parse("Broadcast hello world");
    cr_assert(cmd.has_value());
    cr_assert(cmd->kind == CommandKind::BROADCAST);
    cr_assert_str_eq(cmd->argument.c_str(), "hello world");
}

/* malformed */

Test(command_parser, argument_required_but_missing_is_rejected)
{
    cr_assert_not(CommandParser::parse("Take").has_value());
    cr_assert_not(CommandParser::parse("Broadcast").has_value());
}

Test(command_parser, argument_given_but_unexpected_is_rejected)
{
    cr_assert_not(CommandParser::parse("Forward extra").has_value());
    cr_assert_not(CommandParser::parse("Look here").has_value());
}

Test(command_parser, unknown_command_is_rejected)
{
    cr_assert_not(CommandParser::parse("Bogus").has_value());
    cr_assert_not(CommandParser::parse("Forwardx").has_value());
}

Test(command_parser, empty_line_is_rejected)
{
    cr_assert_not(CommandParser::parse("").has_value());
    cr_assert_not(CommandParser::parse("   ").has_value());
}

/* whitespace tolerance */

Test(command_parser, surrounding_whitespace_is_trimmed)
{
    const auto cmd = CommandParser::parse("  Forward  ");
    cr_assert(cmd.has_value());
    cr_assert(cmd->kind == CommandKind::FORWARD);
    cr_assert_str_eq(cmd->argument.c_str(), "");
}

Test(command_parser, carriage_return_is_tolerated)
{
    const auto cmd = CommandParser::parse("Forward\r");
    cr_assert(cmd.has_value());
    cr_assert(cmd->kind == CommandKind::FORWARD);
    const auto take = CommandParser::parse("Take food\r");
    cr_assert(take.has_value());
    cr_assert_str_eq(take->argument.c_str(), "food");
}
