/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** A single map cell: resources, players and eggs present
*/

#ifndef SERVER_GAME_TILE_HPP_
    #define SERVER_GAME_TILE_HPP_

    #include <vector>

    #include "server/game/Biome.hpp"
    #include "server/game/Constants.hpp"

namespace zappy::server::game {

/**
 * @brief One cell of the toroidal map: a resource stock plus occupants.
 */
class Tile {
public:
    /**
     * @brief Quantity of @p type currently on the tile.
     *
     * @param type resource to query
     * @return int the stored quantity
     */
    [[nodiscard]] int resource(ResourceType type) const noexcept;

    /**
     * @brief Overwrite the quantity of @p type.
     *
     * @param type resource to set
     * @param qty new quantity (clamped to zero if negative)
     */
    void setResource(ResourceType type, int qty) noexcept;

    /**
     * @brief Add @p qty units of @p type.
     *
     * @param type resource to add
     * @param qty quantity to add (must be >= 0)
     * @return bool false if qty is negative
     */
    bool addResource(ResourceType type, int qty) noexcept;

    /**
     * @brief Remove @p qty units of @p type.
     *
     * @param type resource to remove
     * @param qty quantity to remove (must be >= 0)
     * @return bool false if qty is negative or stock is insufficient
     */
    bool removeResource(ResourceType type, int qty) noexcept;

    /**
     * @brief Ids of players standing on the tile.
     *
     * @return const std::vector<int>& the player id list
     */
    [[nodiscard]] const std::vector<int> &playerIds() const noexcept;

    /**
     * @brief Ids of eggs lying on the tile.
     *
     * @return const std::vector<int>& the egg id list
     */
    [[nodiscard]] const std::vector<int> &eggIds() const noexcept;

    /**
     * @brief Register player @p id as present.
     *
     * @param id player id to add
     */
    void addPlayer(int id);

    /**
     * @brief Unregister player @p id; no-op if absent.
     *
     * @param id player id to remove
     */
    void removePlayer(int id) noexcept;

    /**
     * @brief Register egg @p id as present.
     *
     * @param id egg id to add
     */
    void addEgg(int id);

    /**
     * @brief Unregister egg @p id; no-op if absent.
     *
     * @param id egg id to remove
     */
    void removeEgg(int id) noexcept;

    /**
     * @brief Mark the tile as flooded or dry (a runtime event effect).
     *
     * @param value true to flood, false to dry out
     */
    void setFlooded(bool value) noexcept;

    /**
     * @brief Whether the tile is currently flooded.
     *
     * @return bool true if flooded
     */
    [[nodiscard]] bool isFlooded() const noexcept;

    /**
     * @brief Set the tile's terrain biome.
     *
     * @param biome the biome to assign
     */
    void setBiome(Biome biome) noexcept;

    /**
     * @brief The tile's terrain biome.
     *
     * @return Biome the current biome (PLAIN by default)
     */
    [[nodiscard]] Biome biome() const noexcept;

private:
    Inventory _resources{};
    std::vector<int> _playerIds;
    std::vector<int> _eggIds;
    bool _flooded = false;
    Biome _biome = Biome::PLAIN;
};

} // namespace zappy::server::game

#endif /* !SERVER_GAME_TILE_HPP_ */
