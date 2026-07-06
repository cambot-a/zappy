/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Fork command tests for the AI dispatcher
*/

#include <criterion/criterion.h>
#include <string>
#include <utility>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/Constants.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::EggState;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

constexpr Duration FORK_DURATION{420};
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
 * @brief Records the fork-related notifications the world emits.
 */
class MockObserver : public WorldObserverAdapter {
public:
    void onPlayerForkStarted(int id) override { forkStarted.push_back(id); }
    void onEggAdded(int) override { ++eggAdded; }
    void onTeamSlotsChanged(const std::string &t) override
    {
        slotsChanged.push_back(t);
    }

    std::vector<int> forkStarted;
    std::vector<std::string> slotsChanged;
    int eggAdded = 0;
};

/**
 * @brief Bundles a world, scheduler and dispatcher with five seeded eggs.
 */
struct Fixture {
    FakeClock clock;
    Scheduler sched{clock};
    World world{10, 10, {"team"}, 5};
    std::vector<Reply> replies;
    AiDispatcher disp{world, sched, 100,
        [this](int id, std::string r) { replies.push_back({id, std::move(r)}); }};

    Fixture()
    {
        for (int i = 0; i < 5; ++i)
            world.addEgg("team", Position(i, 0));
    }

    int spawnPlayer()
    {
        return world.addPlayer("team", Position(5, 5), Orientation::NORTH);
    }

    void runFork(int id)
    {
        disp.dispatch(id, "Fork");
        clock.advance(FORK_DURATION);
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

Test(ai_dispatcher_fork, creates_egg_at_player_position)
{
    Fixture fx;
    const int id = fx.spawnPlayer();
    fx.runFork(id);
    const std::string *reply = replyFor(fx.replies, id);
    cr_assert_not_null(reply);
    cr_assert_str_eq(reply->c_str(), "ok");
    cr_assert_eq(fx.world.waitingEggCount("team"), 6);
    const auto &eggs = fx.world.tileAt(Position(5, 5)).eggIds();
    cr_assert_eq(eggs.size(), 1U);
    cr_assert_eq(static_cast<int>(fx.world.egg(eggs.front()).state()),
        static_cast<int>(EggState::WAITING));
}

Test(ai_dispatcher_fork, increments_team_slots_total)
{
    Fixture fx;
    const int id = fx.spawnPlayer();
    cr_assert_eq(fx.world.team("team").slotsTotal(), 5);
    fx.runFork(id);
    cr_assert_eq(fx.world.team("team").slotsTotal(), 6);
}

Test(ai_dispatcher_fork, fork_started_fires_at_scheduling_time)
{
    Fixture fx;
    MockObserver obs;
    fx.world.addObserver(obs);
    const int id = fx.spawnPlayer();
    fx.disp.dispatch(id, "Fork");
    fx.sched.tick();
    cr_assert_eq(obs.forkStarted.size(), 1U);
    cr_assert_eq(obs.forkStarted.front(), id);
    cr_assert_eq(obs.eggAdded, 0);
}

Test(ai_dispatcher_fork, egg_added_fires_at_completion)
{
    Fixture fx;
    MockObserver obs;
    const int id = fx.spawnPlayer();
    fx.world.addObserver(obs);
    fx.runFork(id);
    cr_assert_eq(obs.eggAdded, 1);
}

Test(ai_dispatcher_fork, team_slots_changed_fires_at_completion)
{
    Fixture fx;
    MockObserver obs;
    const int id = fx.spawnPlayer();
    fx.world.addObserver(obs);
    fx.runFork(id);
    cr_assert_eq(obs.slotsChanged.size(), 1U);
    cr_assert_str_eq(obs.slotsChanged.front().c_str(), "team");
}

Test(ai_dispatcher_fork, multiple_forks_accumulate_eggs)
{
    Fixture fx;
    const int id = fx.spawnPlayer();
    fx.disp.dispatch(id, "Fork");
    fx.disp.dispatch(id, "Fork");
    fx.disp.dispatch(id, "Fork");
    for (int i = 0; i < 3; ++i) {
        fx.clock.advance(FORK_DURATION);
        fx.sched.tick();
    }
    cr_assert_eq(fx.world.waitingEggCount("team"), 8);
    cr_assert_eq(fx.world.team("team").slotsTotal(), 8);
}
