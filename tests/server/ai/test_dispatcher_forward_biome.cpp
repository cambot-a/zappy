/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Forward-into-impassable-biome tests for the AI dispatcher (ZAP-52, bonus)
*/

#include <criterion/criterion.h>
#include <string>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/Biome.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"
#include "server/scheduler/Clock.hpp"
#include "server/scheduler/Scheduler.hpp"

using zappy::server::ai::AiDispatcher;
using zappy::server::game::Biome;
using zappy::server::game::Orientation;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::scheduler::Duration;
using zappy::server::scheduler::IClock;
using zappy::server::scheduler::Scheduler;
using zappy::server::scheduler::TimePoint;

namespace {

class FakeClock : public IClock {
public:
    void advance(Duration d) noexcept { _now += d; }
    TimePoint now() const noexcept override { return _now; }

private:
    TimePoint _now{};
};

struct ForwardFixture {
    FakeClock clock;
    Scheduler sched{clock};
    World world{10, 10, {"team"}, 10};
    std::vector<std::string> responses;
    AiDispatcher disp{world, sched, 100,
        [this](int, std::string r) { responses.push_back(std::move(r)); }};

    int spawn(Position pos, Orientation orient = Orientation::NORTH)
    {
        return world.addPlayer("team", pos, orient);
    }

    void forward(int id)
    {
        disp.dispatch(id, "Forward");
        clock.advance(Duration(70));
        sched.tick();
    }
};

} // namespace

Test(forward_biome, into_plain_is_ok)
{
    ForwardFixture fx;
    const int id = fx.spawn(Position(5, 5));

    fx.forward(id);
    cr_assert_str_eq(fx.responses.front().c_str(), "ok");
    cr_assert(fx.world.player(id).position() == Position(5, 4));
}

Test(forward_biome, into_mountain_is_ko)
{
    ForwardFixture fx;
    const int id = fx.spawn(Position(5, 5));

    fx.world.setTileBiome(Position(5, 4), Biome::MOUNTAIN);
    fx.forward(id);
    cr_assert_str_eq(fx.responses.front().c_str(), "ko");
    cr_assert(fx.world.player(id).position() == Position(5, 5));
}

Test(forward_biome, into_peak_is_ko)
{
    ForwardFixture fx;
    const int id = fx.spawn(Position(5, 5));

    fx.world.setTileBiome(Position(5, 4), Biome::PEAK);
    fx.forward(id);
    cr_assert_str_eq(fx.responses.front().c_str(), "ko");
    cr_assert(fx.world.player(id).position() == Position(5, 5));
}

Test(forward_biome, into_snow_plain_is_ok)
{
    ForwardFixture fx;
    const int id = fx.spawn(Position(5, 5));

    fx.world.setTileBiome(Position(5, 4), Biome::SNOW_PLAIN);
    fx.forward(id);
    cr_assert_str_eq(fx.responses.front().c_str(), "ok");
    cr_assert(fx.world.player(id).position() == Position(5, 4));
}

Test(forward_biome, leaving_mountain_is_allowed)
{
    ForwardFixture fx;
    const int id = fx.spawn(Position(5, 5));

    fx.world.setTileBiome(Position(5, 5), Biome::MOUNTAIN);
    fx.forward(id);
    cr_assert_str_eq(fx.responses.front().c_str(), "ok");
    cr_assert(fx.world.player(id).position() == Position(5, 4));
}
