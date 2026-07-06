/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Take and Set command tests for the AI dispatcher
*/

#include <criterion/criterion.h>
#include <string>
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
using zappy::server::game::RESOURCE_NAMES;
using zappy::server::game::ResourceType;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

constexpr Duration TAKE_SET_DURATION{70};
constexpr Position PLAYER_POS{5, 5};

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
 * @brief Flags the mutation notifications the world may emit.
 */
class MockObserver : public WorldObserverAdapter {
public:
    void onTileChanged(Position) override { tileChanged = true; }
    void onPlayerInventoryChanged(int) override { inventoryChanged = true; }

    bool tileChanged = false;
    bool inventoryChanged = false;
};

/**
 * @brief Bundles a world, scheduler and dispatcher around one player.
 */
struct Fixture {
    FakeClock clock;
    Scheduler sched{clock};
    World world{10, 10, {"team"}, 10};
    std::vector<std::string> responses;
    AiDispatcher disp{world, sched, 100,
        [this](int, std::string r) { responses.push_back(std::move(r)); }};
    int id = world.addPlayer("team", PLAYER_POS, Orientation::NORTH);

    void run(const std::string &line)
    {
        disp.dispatch(id, line);
        clock.advance(TAKE_SET_DURATION);
        sched.tick();
    }
};

} // namespace

/* 1. taking food present on the tile moves one unit into the inventory */

Test(ai_dispatcher_take_set, take_food_present)
{
    Fixture fix;
    fix.world.setTileResource(PLAYER_POS, ResourceType::FOOD, 3);
    fix.run("Take food");
    cr_assert_str_eq(fix.responses.front().c_str(), "ok");
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::FOOD), 2);
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 11);
}

/* 2. taking food absent from the tile fails and mutates nothing */

Test(ai_dispatcher_take_set, take_food_absent)
{
    Fixture fix;
    fix.run("Take food");
    cr_assert_str_eq(fix.responses.front().c_str(), "ko");
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::FOOD), 0);
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 10);
}

/* 3. taking an unknown resource name fails without mutating */

Test(ai_dispatcher_take_set, take_invalid_name)
{
    Fixture fix;
    fix.world.setTileResource(PLAYER_POS, ResourceType::FOOD, 3);
    fix.run("Take stone");
    cr_assert_str_eq(fix.responses.front().c_str(), "ko");
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::FOOD), 3);
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 10);
}

/* 4. a successful take notifies, a failed take stays silent */

Test(ai_dispatcher_take_set, take_observer_events)
{
    Fixture fix;
    MockObserver obs;
    fix.world.addObserver(obs);
    fix.world.setTileResource(PLAYER_POS, ResourceType::FOOD, 1);
    fix.run("Take food");
    cr_assert(obs.tileChanged);
    cr_assert(obs.inventoryChanged);

    Fixture miss;
    MockObserver missObs;
    miss.world.addObserver(missObs);
    miss.run("Take food");
    cr_assert(!missObs.tileChanged);
    cr_assert(!missObs.inventoryChanged);
}

/* 5. setting food drops one unit from the inventory onto the tile */

Test(ai_dispatcher_take_set, set_food_drops)
{
    Fixture fix;
    fix.run("Set food");
    cr_assert_str_eq(fix.responses.front().c_str(), "ok");
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 9);
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::FOOD), 1);
}

/* 6. setting a resource absent from the inventory fails without mutating */

Test(ai_dispatcher_take_set, set_not_in_inventory)
{
    Fixture fix;
    fix.run("Set linemate");
    cr_assert_str_eq(fix.responses.front().c_str(), "ko");
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::LINEMATE), 0);
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::LINEMATE), 0);
}

/* 7. setting an unknown resource name fails without mutating */

Test(ai_dispatcher_take_set, set_invalid_name)
{
    Fixture fix;
    fix.run("Set bogus");
    cr_assert_str_eq(fix.responses.front().c_str(), "ko");
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 10);
}

/* 8. a successful set notifies tile and inventory */

Test(ai_dispatcher_take_set, set_observer_events)
{
    Fixture fix;
    MockObserver obs;
    fix.world.addObserver(obs);
    fix.run("Set food");
    cr_assert(obs.tileChanged);
    cr_assert(obs.inventoryChanged);
}

/* 9. every resource name round-trips through Take and Set */

Test(ai_dispatcher_take_set, all_names_take_and_set)
{
    for (std::size_t index = 0; index < zappy::server::game::RESOURCE_COUNT;
        ++index) {
        Fixture fix;
        const auto type = static_cast<ResourceType>(index);
        const std::string name{RESOURCE_NAMES[index]};
        fix.world.setTileResource(PLAYER_POS, type, 1);
        fix.world.player(fix.id).addResource(type, 1);
        fix.run("Take " + name);
        fix.run("Set " + name);
        cr_assert_str_eq(fix.responses[0].c_str(), "ok");
        cr_assert_str_eq(fix.responses[1].c_str(), "ok");
    }
}

/* 10. taking then setting food returns the world to its initial state */

Test(ai_dispatcher_take_set, take_then_set_roundtrip)
{
    Fixture fix;
    fix.world.setTileResource(PLAYER_POS, ResourceType::FOOD, 1);
    fix.run("Take food");
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::FOOD), 0);
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 11);
    fix.run("Set food");
    cr_assert_eq(
        fix.world.tileAt(PLAYER_POS).resource(ResourceType::FOOD), 1);
    cr_assert_eq(fix.world.player(fix.id).resource(ResourceType::FOOD), 10);
}
