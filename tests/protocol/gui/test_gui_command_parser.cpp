/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the GUI command parser
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

Test(gui_command_parser, all_9_valid_commands_parse)
{
    // msz
    auto res_msz = GuiCommandParser::parse("msz");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_msz));
    cr_assert(std::get<ParsedGuiCommand>(res_msz).kind == GuiCommandKind::MSZ);
    cr_assert(std::get<ParsedGuiCommand>(res_msz).arguments.empty());

    // bct 3 5
    auto res_bct = GuiCommandParser::parse("bct 3 5");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_bct));
    cr_assert(std::get<ParsedGuiCommand>(res_bct).kind == GuiCommandKind::BCT);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments.size(), 2U);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments[0], 3);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments[1], 5);

    // mct
    auto res_mct = GuiCommandParser::parse("mct");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_mct));
    cr_assert(std::get<ParsedGuiCommand>(res_mct).kind == GuiCommandKind::MCT);

    // tna
    auto res_tna = GuiCommandParser::parse("tna");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_tna));
    cr_assert(std::get<ParsedGuiCommand>(res_tna).kind == GuiCommandKind::TNA);

    // ppo 7
    auto res_ppo = GuiCommandParser::parse("ppo 7");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_ppo));
    cr_assert(std::get<ParsedGuiCommand>(res_ppo).kind == GuiCommandKind::PPO);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_ppo).arguments[0], 7);

    // plv 7
    auto res_plv = GuiCommandParser::parse("plv 7");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_plv));
    cr_assert(std::get<ParsedGuiCommand>(res_plv).kind == GuiCommandKind::PLV);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_plv).arguments[0], 7);

    // pin 7
    auto res_pin = GuiCommandParser::parse("pin 7");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_pin));
    cr_assert(std::get<ParsedGuiCommand>(res_pin).kind == GuiCommandKind::PIN);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_pin).arguments[0], 7);

    // sgt
    auto res_sgt = GuiCommandParser::parse("sgt");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_sgt));
    cr_assert(std::get<ParsedGuiCommand>(res_sgt).kind == GuiCommandKind::SGT);

    // sst 100
    auto res_sst = GuiCommandParser::parse("sst 100");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_sst));
    cr_assert(std::get<ParsedGuiCommand>(res_sst).kind == GuiCommandKind::SST);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_sst).arguments[0], 100);
}

Test(gui_command_parser, ppf_parses_with_and_without_hash)
{
    auto withHash = GuiCommandParser::parse("ppf #5");
    auto noHash = GuiCommandParser::parse("ppf 5");

    cr_assert(std::holds_alternative<ParsedGuiCommand>(withHash));
    cr_assert(std::get<ParsedGuiCommand>(withHash).kind
        == GuiCommandKind::PPF);
    cr_assert_eq(std::get<ParsedGuiCommand>(withHash).arguments[0], 5);
    cr_assert(std::holds_alternative<ParsedGuiCommand>(noHash));
    cr_assert(std::get<ParsedGuiCommand>(noHash).kind == GuiCommandKind::PPF);
    cr_assert_eq(std::get<ParsedGuiCommand>(noHash).arguments[0], 5);
}

Test(gui_command_parser, ppf_invalid_args_give_bad_parameter_error)
{
    std::vector<std::string_view> cases = {"ppf", "ppf #abc", "ppf 1 2"};

    for (const auto &c : cases) {
        auto res = GuiCommandParser::parse(c);
        cr_assert(std::holds_alternative<GuiParseError>(res));
        cr_assert(std::get<GuiParseError>(res)
            == GuiParseError::BAD_PARAMETER);
    }
}

Test(gui_command_parser, unknown_command_gives_unknown_command_error)
{
    std::vector<std::string_view> cases = {"Bogus", "msZ", "", "   ", "\t", "\r", "Msz", "MSZ"};
    for (const auto &c : cases) {
        auto res = GuiCommandParser::parse(c);
        cr_assert(std::holds_alternative<GuiParseError>(res));
        cr_assert(std::get<GuiParseError>(res) == GuiParseError::UNKNOWN_COMMAND);
    }
}

Test(gui_command_parser, wrong_arg_count_gives_bad_parameter_error)
{
    std::vector<std::string_view> cases = {"msz extra", "bct 1", "bct 1 2 3", "ppo", "sst"};
    for (const auto &c : cases) {
        auto res = GuiCommandParser::parse(c);
        cr_assert(std::holds_alternative<GuiParseError>(res));
        cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
    }
}

Test(gui_command_parser, non_integer_arg_gives_bad_parameter_error)
{
    std::vector<std::string_view> cases = {"ppo abc", "bct 1 xyz", "sst 100.5", "sst +100"};
    for (const auto &c : cases) {
        auto res = GuiCommandParser::parse(c);
        cr_assert(std::holds_alternative<GuiParseError>(res));
        cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
    }
}

Test(gui_command_parser, negative_integer_gives_bad_parameter_error)
{
    std::vector<std::string_view> cases = {"ppo -1", "sst -100", "bct -1 2", "bct 1 -2"};
    for (const auto &c : cases) {
        auto res = GuiCommandParser::parse(c);
        cr_assert(std::holds_alternative<GuiParseError>(res));
        cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
    }
}

Test(gui_command_parser, hash_prefix_is_stripped_on_numeric_args)
{
    auto res_ppo = GuiCommandParser::parse("ppo #5");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_ppo));
    cr_assert(std::get<ParsedGuiCommand>(res_ppo).kind == GuiCommandKind::PPO);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_ppo).arguments[0], 5);

    auto res_plain = GuiCommandParser::parse("ppo 5");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_plain));
    cr_assert_eq(std::get<ParsedGuiCommand>(res_plain).arguments[0], 5);

    auto res_bct = GuiCommandParser::parse("bct #3 #5");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_bct));
    cr_assert(std::get<ParsedGuiCommand>(res_bct).kind == GuiCommandKind::BCT);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments[0], 3);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments[1], 5);
}

Test(gui_command_parser, malformed_hash_prefix_gives_bad_parameter_error)
{
    std::vector<std::string_view> cases = {"ppo #abc", "ppo #", "ppo ##5",
        "ppo #-1"};
    for (const auto &c : cases) {
        auto res = GuiCommandParser::parse(c);
        cr_assert(std::holds_alternative<GuiParseError>(res));
        cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
    }
}

Test(gui_command_parser, admin_takes_raw_string_password)
{
    auto res = GuiCommandParser::parse("admin secret");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    cr_assert(std::get<ParsedGuiCommand>(res).kind == GuiCommandKind::ADMIN);
    cr_assert(std::get<ParsedGuiCommand>(res).arguments.empty());
    cr_assert_str_eq(
        std::get<ParsedGuiCommand>(res).rawArgument.c_str(), "secret");
}

Test(gui_command_parser, admin_password_keeps_internal_spaces)
{
    auto res = GuiCommandParser::parse("admin my long password with spaces");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res));
    cr_assert(std::get<ParsedGuiCommand>(res).kind == GuiCommandKind::ADMIN);
    cr_assert_str_eq(std::get<ParsedGuiCommand>(res).rawArgument.c_str(),
        "my long password with spaces");
}

Test(gui_command_parser, admin_without_password_is_bad_parameter)
{
    auto res = GuiCommandParser::parse("admin");
    cr_assert(std::holds_alternative<GuiParseError>(res));
    cr_assert(std::get<GuiParseError>(res) == GuiParseError::BAD_PARAMETER);
}

Test(gui_command_parser, admin_is_case_sensitive)
{
    auto res = GuiCommandParser::parse("ADMIN secret");
    cr_assert(std::holds_alternative<GuiParseError>(res));
    cr_assert(std::get<GuiParseError>(res) == GuiParseError::UNKNOWN_COMMAND);
}

Test(gui_command_parser, whitespace_is_tolerated)
{
    auto res_msz = GuiCommandParser::parse("  msz  ");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_msz));
    cr_assert(std::get<ParsedGuiCommand>(res_msz).kind == GuiCommandKind::MSZ);

    auto res_bct = GuiCommandParser::parse("  bct  3   5  ");
    cr_assert(std::holds_alternative<ParsedGuiCommand>(res_bct));
    cr_assert(std::get<ParsedGuiCommand>(res_bct).kind == GuiCommandKind::BCT);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments[0], 3);
    cr_assert_eq(std::get<ParsedGuiCommand>(res_bct).arguments[1], 5);
}
