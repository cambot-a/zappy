/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for World::setTileBiome and its observer notification
*/

#include <criterion/criterion.h>

#include "server/game/Biome.hpp"
#include "server/game/IWorldObserver.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::game::Biome;
using zappy::server::game::Position;
using zappy::server::game::World;
using zappy::server::game::WorldObserverAdapter;

namespace {

class BiomeObserver : public WorldObserverAdapter {
public:
    void onTileBiomeChanged(Position pos, Biome biome) override
    {
        fired = true;
        lastPos = pos;
        lastBiome = biome;
    }

    bool fired = false;
    Position lastPos{};
    Biome lastBiome = Biome::PLAIN;
};

} // namespace

Test(world_biome, set_tile_biome_fires_observer)
{
    World world(10, 10, {"team"}, 8);
    BiomeObserver obs;
    world.addObserver(obs);

    world.setTileBiome(Position(3, 4), Biome::MOUNTAIN);
    cr_assert(obs.fired);
    cr_assert(obs.lastPos == Position(3, 4));
    cr_assert_eq(obs.lastBiome, Biome::MOUNTAIN);
}

Test(world_biome, successive_updates_apply)
{
    World world(10, 10, {"team"}, 8);

    world.setTileBiome(Position(2, 2), Biome::MOUNTAIN);
    cr_assert_eq(world.tileAt(Position(2, 2)).biome(), Biome::MOUNTAIN);
    world.setTileBiome(Position(2, 2), Biome::SNOW_PLAIN);
    cr_assert_eq(world.tileAt(Position(2, 2)).biome(), Biome::SNOW_PLAIN);
}
