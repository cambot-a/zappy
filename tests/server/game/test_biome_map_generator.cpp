/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for BiomeMapGenerator (ZAP-52, bonus)
*/

#include <criterion/criterion.h>

#include "server/game/Biome.hpp"
#include "server/game/BiomeMapGenerator.hpp"
#include "server/game/Position.hpp"
#include "server/game/World.hpp"

using zappy::server::game::Biome;
using zappy::server::game::BiomeMapGenerator;
using zappy::server::game::Position;
using zappy::server::game::World;

namespace {

int countNonPlain(const World &world)
{
    int count = 0;

    for (int y = 0; y < world.height(); ++y)
        for (int x = 0; x < world.width(); ++x)
            count += world.tileAt(Position(x, y)).biome() != Biome::PLAIN;
    return count;
}

bool sameBiomes(const World &a, const World &b)
{
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
            if (a.tileAt(Position(x, y)).biome()
                != b.tileAt(Position(x, y)).biome())
                return false;
    return true;
}

} // namespace

Test(biome_map_generator, same_seed_is_deterministic)
{
    World a(10, 10, {"team"}, 8);
    World b(10, 10, {"team"}, 8);
    BiomeMapGenerator genA(42);
    BiomeMapGenerator genB(42);

    genA.generate(a);
    genB.generate(b);
    cr_assert(sameBiomes(a, b));
}

Test(biome_map_generator, generate_paints_non_plain_tiles)
{
    World world(10, 10, {"team"}, 8);
    BiomeMapGenerator gen(7);

    gen.generate(world);
    cr_assert_gt(countNonPlain(world), 0);
}

Test(biome_map_generator, distinct_seeds_can_differ)
{
    World a(20, 20, {"team"}, 8);
    World b(20, 20, {"team"}, 8);
    BiomeMapGenerator genA(1);
    BiomeMapGenerator genB(999);

    genA.generate(a);
    genB.generate(b);
    cr_assert_not(sameBiomes(a, b));
}
