/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Initial egg seeding into a World
*/

#include "server/game/EggSpawner.hpp"

#include "server/game/Position.hpp"
#include "server/game/World.hpp"

zappy::server::game::EggSpawner::EggSpawner(std::uint64_t seed)
    : _rng(seed)
{}

void zappy::server::game::EggSpawner::spawnInitial(World &world)
{
    for (const Team &team : world.teams())
        for (int i = 0; i < team.slotsTotal(); ++i)
            spawnOne(world, team.name());
}

void zappy::server::game::EggSpawner::spawnOne(
    World &world, const std::string &teamName)
{
    std::uniform_int_distribution<int> xDist(0, world.width() - 1);
    std::uniform_int_distribution<int> yDist(0, world.height() - 1);
    world.addEgg(teamName, Position(xDist(_rng), yDist(_rng)));
}
