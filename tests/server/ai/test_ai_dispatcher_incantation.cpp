/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Incantation command tests for the AI dispatcher
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
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

constexpr Duration INCANT_DURATION{3000};
constexpr Duration MOVE_DURATION{70};
constexpr Position TILE{5, 5};
using Reply = std::pair<int, std::string>;

class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

class MockObserver : public WorldObserverAdapter {
public:
    void onIncantationStarted(int initiatorId, int level,
        const std::vector<int> &parts) override
    {
        startedInitiator = initiatorId;
        startedLevel = level;
        startedParticipants = parts;
        ++startedCount;
    }

    void onIncantationEnded(int initiatorId, bool success,
        int newLevel) override
    {
        endedInitiator = initiatorId;
        endedSuccess = success;
        endedLevel = newLevel;
        ++endedCount;
    }

    int startedInitiator = -1;
    int startedLevel = 0;
    std::vector<int> startedParticipants;
    int startedCount = 0;
    int endedInitiator = -1;
    bool endedSuccess = false;
    int endedLevel = -1;
    int endedCount = 0;
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

    void giveStones(const std::vector<std::pair<ResourceType, int>> &stones)
    {
        for (const auto &entry : stones)
            world.setTileResource(TILE, entry.first, entry.second);
    }

    void finishIncantation()
    {
        clock.advance(INCANT_DURATION);
        sched.tick();
    }
};

int countReply(const std::vector<Reply> &replies, int id,
    const std::string &text)
{
    int n = 0;
    for (const auto &reply : replies)
        if (reply.first == id && reply.second == text)
            ++n;
    return n;
}

bool hasReply(const std::vector<Reply> &replies, int id,
    const std::string &text)
{
    return countReply(replies, id, text) > 0;
}

} // namespace

Test(ai_dispatcher_incantation, solo_level_one_succeeds)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    fx.disp.dispatch(id, "Incantation");
    cr_assert(hasReply(fx.replies, id, "Elevation underway"));
    fx.finishIncantation();
    cr_assert(hasReply(fx.replies, id, "Current level: 2"));
    cr_assert_eq(fx.world.player(id).level(), 2);
    cr_assert_eq(fx.world.tileAt(TILE).resource(ResourceType::LINEMATE), 0);
}

Test(ai_dispatcher_incantation, solo_level_one_without_stone_kos)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.disp.dispatch(id, "Incantation");
    cr_assert(hasReply(fx.replies, id, "ko"));
    cr_assert(!hasReply(fx.replies, id, "Elevation underway"));
    cr_assert_eq(fx.world.player(id).level(), 1);
    cr_assert_eq(fx.sched.pendingCount(), 0U);
}

Test(ai_dispatcher_incantation, solo_level_two_lacks_players_kos)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.world.setPlayerLevel(id, 2);
    fx.giveStones({{ResourceType::LINEMATE, 1}, {ResourceType::DERAUMERE, 1},
        {ResourceType::SIBUR, 1}});
    fx.disp.dispatch(id, "Incantation");
    cr_assert(hasReply(fx.replies, id, "ko"));
    cr_assert_eq(fx.world.player(id).level(), 2);
}

Test(ai_dispatcher_incantation, two_players_level_two_succeed)
{
    Fixture fx;
    const int a = fx.spawn();
    const int b = fx.spawn();
    fx.world.setPlayerLevel(a, 2);
    fx.world.setPlayerLevel(b, 2);
    fx.giveStones({{ResourceType::LINEMATE, 1}, {ResourceType::DERAUMERE, 1},
        {ResourceType::SIBUR, 1}});
    fx.disp.dispatch(a, "Incantation");
    cr_assert(hasReply(fx.replies, a, "Elevation underway"));
    cr_assert(hasReply(fx.replies, b, "Elevation underway"));
    fx.finishIncantation();
    cr_assert_eq(fx.world.player(a).level(), 3);
    cr_assert_eq(fx.world.player(b).level(), 3);
    cr_assert(hasReply(fx.replies, a, "Current level: 3"));
    cr_assert(hasReply(fx.replies, b, "Current level: 3"));
    cr_assert_eq(fx.world.tileAt(TILE).resource(ResourceType::LINEMATE), 0);
    cr_assert_eq(fx.world.tileAt(TILE).resource(ResourceType::SIBUR), 0);
}

Test(ai_dispatcher_incantation, participant_is_frozen_during_ritual)
{
    Fixture fx;
    const int a = fx.spawn();
    const int b = fx.spawn();
    fx.world.setPlayerLevel(a, 2);
    fx.world.setPlayerLevel(b, 2);
    fx.giveStones({{ResourceType::LINEMATE, 1}, {ResourceType::DERAUMERE, 1},
        {ResourceType::SIBUR, 1}});
    fx.disp.dispatch(a, "Incantation");
    fx.disp.dispatch(b, "Forward");
    fx.clock.advance(MOVE_DURATION);
    fx.sched.tick();
    cr_assert_eq(fx.world.player(b).position().x(), TILE.x());
    cr_assert_eq(fx.world.player(b).position().y(), TILE.y());
    fx.clock.advance(INCANT_DURATION - MOVE_DURATION);
    fx.sched.tick();
    cr_assert_eq(fx.world.player(b).level(), 3);
    fx.clock.advance(MOVE_DURATION);
    fx.sched.tick();
    const bool moved = fx.world.player(b).position().x() != TILE.x()
        || fx.world.player(b).position().y() != TILE.y();
    cr_assert(moved);
}

Test(ai_dispatcher_incantation, participant_death_fails_ritual)
{
    Fixture fx;
    const int a = fx.spawn();
    const int b = fx.spawn();
    fx.world.setPlayerLevel(a, 2);
    fx.world.setPlayerLevel(b, 2);
    fx.giveStones({{ResourceType::LINEMATE, 1}, {ResourceType::DERAUMERE, 1},
        {ResourceType::SIBUR, 1}});
    fx.disp.dispatch(a, "Incantation");
    fx.world.killPlayer(b);
    fx.finishIncantation();
    cr_assert(hasReply(fx.replies, a, "ko"));
    cr_assert_eq(fx.world.player(a).level(), 2);
    cr_assert_eq(fx.world.tileAt(TILE).resource(ResourceType::LINEMATE), 1);
}

Test(ai_dispatcher_incantation, external_stone_removal_fails_ritual)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    fx.disp.dispatch(id, "Incantation");
    fx.world.setTileResource(TILE, ResourceType::LINEMATE, 0);
    fx.finishIncantation();
    cr_assert(hasReply(fx.replies, id, "ko"));
    cr_assert_eq(fx.world.player(id).level(), 1);
}

Test(ai_dispatcher_incantation, observer_sees_start_and_success)
{
    Fixture fx;
    MockObserver obs;
    fx.world.addObserver(obs);
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    fx.disp.dispatch(id, "Incantation");
    cr_assert_eq(obs.startedCount, 1);
    cr_assert_eq(obs.startedInitiator, id);
    cr_assert_eq(obs.startedLevel, 1);
    cr_assert_eq(obs.startedParticipants.size(), 1U);
    cr_assert_eq(obs.endedCount, 0);
    fx.finishIncantation();
    cr_assert_eq(obs.endedCount, 1);
    cr_assert(obs.endedSuccess);
    cr_assert_eq(obs.endedLevel, 2);
}

Test(ai_dispatcher_incantation, observer_sees_failure)
{
    Fixture fx;
    MockObserver obs;
    fx.world.addObserver(obs);
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    fx.disp.dispatch(id, "Incantation");
    fx.world.setTileResource(TILE, ResourceType::LINEMATE, 0);
    fx.finishIncantation();
    cr_assert_eq(obs.endedCount, 1);
    cr_assert(!obs.endedSuccess);
    cr_assert_eq(obs.endedLevel, 0);
}

Test(ai_dispatcher_incantation, level_eight_cannot_incant)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.world.setPlayerLevel(id, 8);
    fx.disp.dispatch(id, "Incantation");
    cr_assert(hasReply(fx.replies, id, "ko"));
    cr_assert(!hasReply(fx.replies, id, "Elevation underway"));
}

Test(ai_dispatcher_incantation, queued_commands_resume_after_ritual)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    fx.disp.dispatch(id, "Incantation");
    fx.disp.dispatch(id, "Forward");
    fx.disp.dispatch(id, "Look");
    fx.finishIncantation();
    cr_assert_eq(fx.world.player(id).level(), 2);
    fx.clock.advance(MOVE_DURATION);
    fx.sched.tick();
    cr_assert(hasReply(fx.replies, id, "ok"));
    fx.clock.advance(MOVE_DURATION);
    fx.sched.tick();
    const bool moved = fx.world.player(id).position().y() != TILE.y()
        || fx.world.player(id).position().x() != TILE.x();
    cr_assert(moved);
}

Test(ai_dispatcher_incantation, admin_start_runs_full_ritual)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    cr_assert(fx.disp.adminStartIncantation(id));
    cr_assert(hasReply(fx.replies, id, "Elevation underway"));
    fx.finishIncantation();
    cr_assert(hasReply(fx.replies, id, "Current level: 2"));
    cr_assert_eq(fx.world.player(id).level(), 2);
}

Test(ai_dispatcher_incantation, admin_start_without_stone_forces)
{
    Fixture fx;
    const int id = fx.spawn();
    // Admin force ignores the stone/headcount prerequisites.
    cr_assert(fx.disp.adminStartIncantation(id));
    cr_assert(hasReply(fx.replies, id, "Elevation underway"));
    fx.finishIncantation();
    cr_assert(hasReply(fx.replies, id, "Current level: 2"));
    cr_assert_eq(fx.world.player(id).level(), 2);
}

Test(ai_dispatcher_incantation, admin_stop_aborts_ongoing_ritual)
{
    Fixture fx;
    const int id = fx.spawn();
    fx.giveStones({{ResourceType::LINEMATE, 1}});
    cr_assert(fx.disp.adminStartIncantation(id));
    cr_assert(fx.disp.adminStopIncantation(id));
    cr_assert(hasReply(fx.replies, id, "ko"));
    cr_assert_eq(fx.world.player(id).level(), 1);
    fx.finishIncantation();
    cr_assert_not(hasReply(fx.replies, id, "Current level: 2"));
    cr_assert_eq(fx.world.player(id).level(), 1);
}

Test(ai_dispatcher_incantation, admin_stop_without_ritual_fails)
{
    Fixture fx;
    const int id = fx.spawn();
    cr_assert_not(fx.disp.adminStopIncantation(id));
}
