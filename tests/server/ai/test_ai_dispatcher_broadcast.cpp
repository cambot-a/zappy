/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Broadcast command tests for the AI dispatcher
*/

#include <criterion/criterion.h>
#include <string>
#include <utility>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/Constants.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

constexpr Duration BROADCAST_DURATION{70};
using Reply = std::pair<int, std::string>;

/**
 * @brief Test clock whose time only moves when the test advances it.
 */
class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

/**
 * @brief Records every broadcast notification the world emits.
 */
class MockObserver : public WorldObserverAdapter {
public:
    void onPlayerBroadcast(int id, const std::string &text) override
    {
        broadcasts.push_back({id, text});
    }

    std::vector<Reply> broadcasts;
};

/**
 * @brief Bundles a world, scheduler and dispatcher capturing replies.
 */
struct Fixture {
    FakeClock clock;
    Scheduler sched{clock};
    World world{10, 10, {"team"}, 5};
    std::vector<Reply> replies;
    AiDispatcher disp{world, sched, 100,
        [this](int id, std::string r) { replies.push_back({id, std::move(r)}); }};

    int addPlayer(Position pos, Orientation orient)
    {
        return world.addPlayer("team", pos, orient);
    }

    void run(int id, const std::string &line)
    {
        disp.dispatch(id, line);
        clock.advance(BROADCAST_DURATION);
        sched.tick();
    }
};

const std::string *replyFor(const std::vector<Reply> &replies, int id)
{
    for (const auto &reply : replies)
        if (reply.first == id)
            return &reply.second;
    return nullptr;
}

} // namespace

/* 1. a lone sender only gets its own "ok" */

Test(ai_dispatcher_broadcast, sender_alone)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    fix.run(a, "Broadcast hello");
    cr_assert_eq(fix.replies.size(), 1U);
    cr_assert_eq(fix.replies.front().first, a);
    cr_assert_str_eq(fix.replies.front().second.c_str(), "ok");
}

/* 2. a receiver on the sender's tile hears K=0 */

Test(ai_dispatcher_broadcast, same_tile_k0)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    const int b = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    fix.run(a, "Broadcast hi");
    cr_assert_str_eq(replyFor(fix.replies, a)->c_str(), "ok");
    cr_assert_str_eq(replyFor(fix.replies, b)->c_str(), "message 0, hi");
}

/* 3. sender behind a NORTH-facing receiver is heard at K=5 */

Test(ai_dispatcher_broadcast, behind_receiver_k5)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    const int b = fix.addPlayer(Position(5, 4), Orientation::NORTH);
    fix.run(a, "Broadcast yo");
    cr_assert_str_eq(replyFor(fix.replies, a)->c_str(), "ok");
    cr_assert_str_eq(replyFor(fix.replies, b)->c_str(), "message 5, yo");
}

/* 4. several receivers each hear their own K */

Test(ai_dispatcher_broadcast, multiple_receivers)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    const int b = fix.addPlayer(Position(5, 4), Orientation::NORTH);
    const int c = fix.addPlayer(Position(4, 5), Orientation::NORTH);
    fix.run(a, "Broadcast x");
    cr_assert_str_eq(replyFor(fix.replies, b)->c_str(), "message 5, x");
    cr_assert_str_eq(replyFor(fix.replies, c)->c_str(), "message 3, x");
}

/* 5. a dead player never receives a broadcast */

Test(ai_dispatcher_broadcast, dead_player_silent)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    const int b = fix.addPlayer(Position(5, 4), Orientation::NORTH);
    fix.world.killPlayer(b);
    fix.run(a, "Broadcast x");
    cr_assert_eq(fix.replies.size(), 1U);
    cr_assert_eq(fix.replies.front().first, a);
}

/* 6. a broadcast fires onPlayerBroadcast exactly once with the text */

Test(ai_dispatcher_broadcast, observer_event)
{
    Fixture fix;
    MockObserver obs;
    fix.world.addObserver(obs);
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    fix.run(a, "Broadcast hello");
    cr_assert_eq(obs.broadcasts.size(), 1U);
    cr_assert_eq(obs.broadcasts.front().first, a);
    cr_assert_str_eq(obs.broadcasts.front().second.c_str(), "hello");
}

/* 7. broadcast text passes through spaces and non-ASCII intact */

Test(ai_dispatcher_broadcast, text_with_spaces)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    const int b = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    fix.run(a, "Broadcast hello world, ça va?");
    cr_assert_str_eq(replyFor(fix.replies, b)->c_str(),
        "message 0, hello world, ça va?");
}

/* 8. the broadcast resolves at exactly 7/f, not before */

Test(ai_dispatcher_broadcast, duration_seven_over_f)
{
    Fixture fix;
    const int a = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    const int b = fix.addPlayer(Position(5, 5), Orientation::NORTH);
    fix.disp.dispatch(a, "Broadcast hi");
    fix.clock.advance(Duration(69));
    fix.sched.tick();
    cr_assert(fix.replies.empty());
    fix.clock.advance(Duration(1));
    fix.sched.tick();
    cr_assert_eq(fix.replies.size(), 2U);
    cr_assert_not_null(replyFor(fix.replies, b));
}
