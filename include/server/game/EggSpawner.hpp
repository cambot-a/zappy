/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Initial egg seeding into a World
*/

#ifndef SERVER_GAME_EGGSPAWNER_HPP_
    #define SERVER_GAME_EGGSPAWNER_HPP_

    #include <cstdint>
    #include <random>
    #include <string>

namespace zappy::server::game {

class World;

/**
 * @brief Seeds the map with the initial team eggs at server boot.
 */
class EggSpawner {
public:
    /**
     * @brief Build a spawner with a seeded random engine.
     *
     * @param seed value feeding the random engine
     */
    explicit EggSpawner(std::uint64_t seed = std::random_device{}());

    /**
     * @brief Lay slotsTotal eggs per team over random tiles.
     *
     * @param world world to seed in place
     */
    void spawnInitial(World &world);

private:
    /**
     * @brief Lay one egg for @p teamName on a random tile.
     *
     * @param world world to seed in place
     * @param teamName owning team name
     */
    void spawnOne(World &world, const std::string &teamName);

    std::mt19937_64 _rng;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_EGGSPAWNER_HPP_ */
