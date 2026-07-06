/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for parsing admin (adm_*) GUI commands, incl. mixed int+raw args
*/

#include <criterion/criterion.h>
#include <variant>
#include <vector>

#include "protocol/gui/GuiCommandKind.hpp"
#include "protocol/gui/GuiCommandParser.hpp"

using zappy::protocol::gui::GuiCommandKind;
using zappy::protocol::gui::GuiCommandParser;
using zappy::protocol::gui::ParsedGuiCommand;
using zappy::protocol::gui::GuiParseError;

Test(gui_command_parser_admin, flag_enable_keeps_raw_name)
{
    auto res = GuiCommandParser::parse("adm_flag_enable biomes");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    const auto &cmd = std::get<ParsedGuiCommand>(res);
    cr_assert(cmd.kind == GuiCommandKind::ADM_FLAG_ENABLE);
    cr_assert(cmd.arguments.empty());
    cr_assert_str_eq(cmd.rawArgument.c_str(), "biomes");
}

Test(gui_command_parser_admin, flag_list_takes_no_argument)
{
    auto res = GuiCommandParser::parse("adm_flag_list");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    cr_assert(std::get<ParsedGuiCommand>(res).kind
        == GuiCommandKind::ADM_FLAG_LIST);
}

Test(gui_command_parser_admin, tile_set_splits_three_ints_then_raw)
{
    auto res = GuiCommandParser::parse("adm_tile_set 3 4 5 food");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    const auto &cmd = std::get<ParsedGuiCommand>(res);
    cr_assert(cmd.kind == GuiCommandKind::ADM_TILE_SET);
    cr_assert_eq(cmd.arguments.size(), 3U);
    cr_assert_eq(cmd.arguments[0], 3);
    cr_assert_eq(cmd.arguments[1], 4);
    cr_assert_eq(cmd.arguments[2], 5);
    cr_assert_str_eq(cmd.rawArgument.c_str(), "food");
}

Test(gui_command_parser_admin, tile_set_missing_raw_is_bad_parameter)
{
    auto res = GuiCommandParser::parse("adm_tile_set 3 4 5");

    cr_assert(std::holds_alternative<GuiParseError>(res));
    cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
}

Test(gui_command_parser_admin, tile_set_non_integer_is_bad_parameter)
{
    auto res = GuiCommandParser::parse("adm_tile_set 3 x 5 food");

    cr_assert(std::holds_alternative<GuiParseError>(res));
    cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
}

Test(gui_command_parser_admin, player_kill_strips_hash_prefix)
{
    auto res = GuiCommandParser::parse("adm_player_kill #7");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    const auto &cmd = std::get<ParsedGuiCommand>(res);
    cr_assert(cmd.kind == GuiCommandKind::ADM_PLAYER_KILL);
    cr_assert_eq(cmd.arguments.size(), 1U);
    cr_assert_eq(cmd.arguments[0], 7);
}

Test(gui_command_parser_admin, player_kill_extra_token_is_bad_parameter)
{
    auto res = GuiCommandParser::parse("adm_player_kill #7 8");

    cr_assert(std::holds_alternative<GuiParseError>(res));
    cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
}

Test(gui_command_parser_admin, player_tp_parses_three_ints)
{
    auto res = GuiCommandParser::parse("adm_player_tp #1 7 8");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    const auto &cmd = std::get<ParsedGuiCommand>(res);
    cr_assert(cmd.kind == GuiCommandKind::ADM_PLAYER_TP);
    cr_assert_eq(cmd.arguments.size(), 3U);
    cr_assert_eq(cmd.arguments[0], 1);
    cr_assert_eq(cmd.arguments[1], 7);
    cr_assert_eq(cmd.arguments[2], 8);
}

Test(gui_command_parser_admin, event_trigger_preserves_spaces_in_raw)
{
    auto res = GuiCommandParser::parse("adm_event_trigger big storm");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    cr_assert_str_eq(
        std::get<ParsedGuiCommand>(res).rawArgument.c_str(), "big storm");
}
