/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Criterion unit tests for the zappy_server CLI parser
*/

#include <criterion/criterion.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "server/cli/CliParseError.hpp"
#include "server/cli/CliParser.hpp"
#include "server/cli/ServerConfig.hpp"

namespace {

/// @brief Run the parser on a literal argument list (argv[0] prepended).
std::optional<zappy::server::cli::ServerConfig> run(
    std::vector<const char *> args)
{
    args.insert(args.begin(), "./zappy_server");
    zappy::server::cli::CliParser parser;
    return parser.parse(static_cast<int>(args.size()), args.data());
}

}

/* valid */

Test(cli_valid, minimal_command_line)
{
    auto config = run({"-p", "4242", "-x", "10", "-y", "20",
        "-n", "blue", "red", "-c", "5"});

    cr_assert(config.has_value(), "minimal valid command line should parse");
    cr_assert_eq(config->port(), 4242);
    cr_assert_eq(config->width(), 10);
    cr_assert_eq(config->height(), 20);
    cr_assert_eq(config->clientsPerTeam(), 5);
    cr_assert_eq(config->teamNames().size(), 2u);
    cr_assert_str_eq(config->teamNames()[0].c_str(), "blue");
    cr_assert_str_eq(config->teamNames()[1].c_str(), "red");
}

Test(cli_valid, frequency_defaults_to_100)
{
    auto config = run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1"});

    cr_assert(config.has_value());
    cr_assert_eq(config->frequency(), 100, "freq should default to 100");
}

Test(cli_valid, explicit_frequency_is_kept)
{
    auto config = run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1",
        "-f", "42"});

    cr_assert(config.has_value());
    cr_assert_eq(config->frequency(), 42);
}

Test(cli_valid, all_flags_including_bonus)
{
    auto config = run({"-p", "8080", "-x", "30", "-y", "30",
        "-n", "alpha", "beta", "gamma", "-c", "8", "-f", "200",
        "--enable-events", "--enable-biomes", "--enable-admin",
        "--admin-password", "s3cr3t"});

    cr_assert(config.has_value());
    cr_assert_eq(config->teamNames().size(), 3u);
    cr_assert(config->eventsEnabled());
    cr_assert(config->biomesEnabled());
    cr_assert(config->adminEnabled());
    cr_assert_str_eq(config->adminPassword().c_str(), "s3cr3t");
}

Test(cli_valid, bonus_flags_default_to_disabled)
{
    auto config = run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1"});

    cr_assert(config.has_value());
    cr_assert_not(config->eventsEnabled());
    cr_assert_not(config->biomesEnabled());
    cr_assert_not(config->adminEnabled());
    cr_assert_str_empty(config->adminPassword().c_str());
}

Test(cli_valid, admin_password_without_enable_is_stored_and_unused)
{
    auto config = run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1",
        "--admin-password", "lonely"});

    cr_assert(config.has_value());
    cr_assert_not(config->adminEnabled());
    cr_assert_str_eq(config->adminPassword().c_str(), "lonely");
}

Test(cli_error, enable_admin_without_password_throws)
{
    cr_assert_throw(
        run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1",
            "--enable-admin"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, enable_admin_with_empty_password_throws)
{
    cr_assert_throw(
        run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1",
            "--enable-admin", "--admin-password", ""}),
        zappy::server::cli::CliParseError);
}

/* help */

Test(cli_help, returns_nullopt)
{
    auto config = run({"--help"});

    cr_assert_not(config.has_value(), "--help must not produce a config");
}

Test(cli_help, usage_message_matches_subject)
{
    const std::string usage = zappy::server::cli::CliParser::usage();

    cr_assert(usage.rfind(
        "USAGE: ./zappy_server -p port -x width -y height "
        "-n name1 name2 ... -c clientsNb -f freq", 0) == 0,
        "usage must start with the subject header");
    cr_assert(usage.find("-p port      port number") != std::string::npos);
    cr_assert(usage.find("reciprocal of time unit") != std::string::npos);
}

/* error */

Test(cli_valid, no_arguments_uses_all_defaults)
{
    auto config = run({});

    cr_assert(config.has_value(), "no-arg launch should parse with defaults");
    cr_assert_eq(config->port(), 4242);
    cr_assert_eq(config->width(), 10);
    cr_assert_eq(config->height(), 10);
    cr_assert_eq(config->clientsPerTeam(), 5);
    cr_assert_eq(config->frequency(), 100);
    cr_assert_eq(config->teamNames().size(), 2u);
    cr_assert_str_eq(config->teamNames()[0].c_str(), "team1");
    cr_assert_str_eq(config->teamNames()[1].c_str(), "team2");
}

Test(cli_valid, omitted_port_falls_back_to_default)
{
    auto config = run({"-x", "1", "-y", "1", "-n", "a", "-c", "1"});

    cr_assert(config.has_value());
    cr_assert_eq(config->port(), 4242, "missing -p should default to 4242");
}

Test(cli_error, missing_value_for_flag)
{
    cr_assert_throw(run({"-p"}), zappy::server::cli::CliParseError);
}

Test(cli_error, negative_port)
{
    cr_assert_throw(run({"-p", "-5", "-x", "1", "-y", "1", "-n", "a", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, port_out_of_range)
{
    cr_assert_throw(
        run({"-p", "70000", "-x", "1", "-y", "1", "-n", "a", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, zero_width)
{
    cr_assert_throw(run({"-p", "1", "-x", "0", "-y", "1", "-n", "a", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, non_numeric_value)
{
    cr_assert_throw(
        run({"-p", "abc", "-x", "1", "-y", "1", "-n", "a", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, trailing_garbage_in_number)
{
    cr_assert_throw(
        run({"-p", "42x", "-x", "1", "-y", "1", "-n", "a", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, duplicate_team_name)
{
    cr_assert_throw(
        run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "a", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, graphic_team_name_is_reserved)
{
    cr_assert_throw(
        run({"-p", "1", "-x", "1", "-y", "1", "-n", "GRAPHIC", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, no_team_names)
{
    cr_assert_throw(run({"-p", "1", "-x", "1", "-y", "1", "-n", "-c", "1"}),
        zappy::server::cli::CliParseError);
}

Test(cli_error, unknown_flag)
{
    cr_assert_throw(
        run({"-p", "1", "-x", "1", "-y", "1", "-n", "a", "-c", "1", "-z"}),
        zappy::server::cli::CliParseError);
}
