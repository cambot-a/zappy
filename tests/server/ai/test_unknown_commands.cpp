/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unknown / malformed command handling tests for the AI dispatcher
*/

#include <criterion/criterion.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/Constants.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

constexpr Duration CMD_DURATION{70};
constexpr Position TILE{5, 5};
using Reply = std::pair<int, std::string>;

class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

struct Fixture {
    FakeClock clock;
    Scheduler sched{clock};
    World world{10, 10, {"team"}, 10};
    std::vector<Reply> replies;
    AiDispatcher disp{world, sched, 100,
        [this](int id, std::string r) {
            replies.push_back({id, std::move(r)});
        }};

    int spawn()
    {
        return world.addPlayer("team", TILE, Orientation::NORTH);
    }

    void step()
    {
        clock.advance(CMD_DURATION);
        sched.tick();
    }
};

int countReply(const std::vector<Reply> &replies, const std::string &text)
{
    int n = 0;
    for (const auto &reply : replies)
        if (reply.second == text)
            ++n;
    return n;
}

const std::string *lastReply(const std::vector<Reply> &replies)
{
    return replies.empty() ? nullptr : &replies.back().second;
}

} // namespace

Test(unknown_commands, random_garbage_returns_immediate_ko)
{
    Fixture fx;
    const int id = fx.spawn();
    for (const char *line : {"Bogus", "ABCDEFG", "12345", "!@#$%^"})
        fx.disp.dispatch(id, line);
    cr_assert_eq(countReply(fx.replies, "ko"), 4);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(unknown_commands, blank_lines_return_immediate_ko)
{
    Fixture fx;
    const int id = fx.spawn();
    for (const char *line : {"", "   ", "\t", "\r"})
        fx.disp.dispatch(id, line);
    cr_assert_eq(countReply(fx.replies, "ko"), 4);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(unknown_commands, missing_required_argument_returns_ko)
{
    Fixture fx;
    const int id = fx.spawn();
    for (const char *line : {"Take", "Set", "Broadcast"})
        fx.disp.dispatch(id, line);
    cr_assert_eq(countReply(fx.replies, "ko"), 3);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(unknown_commands, unexpected_argument_returns_ko)
{
    Fixture fx;
    const int id = fx.spawn();
    for (const char *line : {"Forward extra", "Look something",
        "Inventory whatever", "Connect_nbr 42", "Fork now",
        "Incantation please"})
        fx.disp.dispatch(id, line);
    cr_assert_eq(countReply(fx.replies, "ko"), 6);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(unknown_commands, command_names_are_case_sensitive)
{
    Fixture fx;
    const int id = fx.spawn();
    for (const char *line : {"forward", "FORWARD", "FoRwArD"})
        fx.disp.dispatch(id, line);
    cr_assert_eq(countReply(fx.replies, "ko"), 3);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(unknown_commands, exact_case_command_is_accepted)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.disp.dispatch(id, "Forward");
    cr_assert_eq(countReply(fx.replies, "ko"), 0);
    cr_assert_eq(fx.sched.pendingCount(), 1U);
    fx.step();
    cr_assert_eq(countReply(fx.replies, "ok"), 1);
}

Test(unknown_commands, invalid_resource_name_kos_after_duration)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.disp.dispatch(id, "Take stone");
    cr_assert_eq(fx.sched.pendingCount(), 1U);
    cr_assert_eq(countReply(fx.replies, "ko"), 0);
    fx.step();
    cr_assert_str_eq(lastReply(fx.replies)->c_str(), "ko");
    fx.disp.dispatch(id, "Set crystal");
    fx.step();
    cr_assert_eq(countReply(fx.replies, "ko"), 2);
}

Test(unknown_commands, resource_names_are_case_sensitive_and_trimmed)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.disp.dispatch(id, "Take Food");
    fx.step();
    cr_assert_str_eq(lastReply(fx.replies)->c_str(), "ko");
    fx.disp.dispatch(id, "Take food ");
    fx.step();
    cr_assert_str_eq(lastReply(fx.replies)->c_str(), "ko");
    cr_assert_eq(fx.world.player(id).resource(
        zappy::server::game::ResourceType::FOOD),
        zappy::server::game::INITIAL_FOOD);
}

Test(unknown_commands, player_keeps_working_after_ko)
{
    Fixture fx;
    const int id = fx.spawn();
    for (const char *line : {"Bogus", "Forward", "Junk", "Right", "BadCommand"})
        fx.disp.dispatch(id, line);
    cr_assert_eq(countReply(fx.replies, "ko"), 3);
    cr_assert_eq(countReply(fx.replies, "ok"), 0);
    fx.step();
    cr_assert_eq(countReply(fx.replies, "ok"), 1);
    fx.step();
    cr_assert_eq(countReply(fx.replies, "ok"), 2);
    cr_assert(fx.world.hasPlayer(id));
}

Test(unknown_commands, extreme_inputs_do_not_crash)
{
    Fixture fx;
    const int id = fx.spawn();
    const std::string longName = "Forward" + std::string(200, 'd');
    fx.disp.dispatch(id, longName);
    fx.disp.dispatch(id, "Forward;rm -rf /");
    fx.disp.dispatch(id, std::string_view("Fo\0rward", 8));
    cr_assert_eq(countReply(fx.replies, "ko"), 3);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(unknown_commands, queue_cap_untouched_by_unknown_commands)
{
    Fixture fx;
    const int id = fx.spawn();
    for (int i = 0; i < 15; ++i) {
        fx.disp.dispatch(id, "Garbage");
        cr_assert_eq(fx.sched.pendingCount(), 0U);
    }
    cr_assert_eq(countReply(fx.replies, "ko"), 15);
}

Test(unknown_commands, mix_of_unknown_and_valid_preserves_order)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.disp.dispatch(id, "Bogus");
    fx.disp.dispatch(id, "Forward");
    fx.disp.dispatch(id, "Junk");
    cr_assert_eq(fx.replies.size(), 2U);
    cr_assert_str_eq(fx.replies[0].second.c_str(), "ko");
    cr_assert_str_eq(fx.replies[1].second.c_str(), "ko");
    fx.step();
    cr_assert_eq(fx.replies.size(), 3U);
    cr_assert_str_eq(fx.replies[2].second.c_str(), "ok");
}
