/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Shared utility class to build GUI response lines
*/

#ifndef SERVER_GUI_GUILINEBUILDER_HPP_
    #define SERVER_GUI_GUILINEBUILDER_HPP_

    #include <string>
    #include <vector>
    #include "server/game/World.hpp"

namespace zappy::server::gui {

/**
 * @brief Helper utility class that centralizes the construction of GUI
 *        protocol response and broadcast strings.
 */
class GuiLineBuilder {
public:
    /**
     * @brief Construct a new Gui Line Builder.
     *
     * @param world read-only reference to the game world
     */
    explicit GuiLineBuilder(const game::World &world) noexcept;

    GuiLineBuilder(const GuiLineBuilder &) = default;
    GuiLineBuilder &operator=(const GuiLineBuilder &) = default;
    GuiLineBuilder(GuiLineBuilder &&) noexcept = default;
    GuiLineBuilder &operator=(GuiLineBuilder &&) noexcept = default;

    ~GuiLineBuilder() = default;

    /**
     * @brief Build the "bct X Y q0..q6" line for tile (X, Y).
     *
     * @param x tile horizontal coordinate
     * @param y tile vertical coordinate
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string bct(int x, int y) const;

    /**
     * @brief Build the "ppo #n X Y O" line for player n.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string ppo(int playerId) const;

    /**
     * @brief Build the "plv #n L" line for player n.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string plv(int playerId) const;

    /**
     * @brief Build the "pin #n X Y q0..q6" line for player n.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pin(int playerId) const;

    /**
     * @brief Build the "pnw #n X Y O L N" line for player n.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pnw(int playerId) const;

    /**
     * @brief Build the "pex #n" line for player ejection.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pex(int playerId) const;

    /**
     * @brief Build the "pbc #n M" line for player broadcast.
     *
     * @param playerId player ID
     * @param message broadcast text
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pbc(int playerId, const std::string &message) const;

    /**
     * @brief Build the "pic X Y L #n #n ..." line for incantation start.
     *
     * @param initiatorId initiator player ID
     * @param level current level
     * @param participants participating players
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pic(int initiatorId, int level,
        const std::vector<int> &participants) const;

    /**
     * @brief Build the "pie X Y R" line for incantation end.
     *
     * @param pos tile position
     * @param success whether it succeeded
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pie(game::Position pos, bool success) const;

    /**
     * @brief Build the "pfk #n" line for player fork start.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pfk(int playerId) const;

    /**
     * @brief Build the "pdr #n i" line for player dropping a resource.
     *
     * @param playerId player ID
     * @param resource type of resource dropped
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pdr(int playerId, game::ResourceType resource) const;

    /**
     * @brief Build the "pgt #n i" line for player picking up a resource.
     *
     * @param playerId player ID
     * @param resource type of resource picked up
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string pgt(int playerId, game::ResourceType resource) const;

    [[nodiscard]] std::string pdi(int playerId) const;

    /**
     * @brief Build the "ppf #n N X Y O L q0..q6" aggregated profile line.
     *
     * @param playerId player ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string ppf(int playerId) const;

    /**
     * @brief Build the "enw #e #n X Y" line for a laid egg.
     *
     * @param eggId egg ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string enw(int eggId) const;

    /**
     * @brief Build the "ebo #e" line for egg used by AI connection.
     *
     * @param eggId egg ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string ebo(int eggId) const;

    /**
     * @brief Build the "edi #e" line for egg death.
     *
     * @param eggId egg ID
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string edi(int eggId) const;

    /**
     * @brief Build the "seg N" line for end of game.
     *
     * @param teamName name of the winning team
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string seg(const std::string &teamName) const;

    /**
     * @brief Build the "smg M" line for a server message.
     *
     * @param message the message string
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string smg(const std::string &message) const;

    /**
     * @brief Build the "evt_flood_tile X Y on|off" custom flood line.
     *
     * @param x tile horizontal coordinate
     * @param y tile vertical coordinate
     * @param flooded true for "on", false for "off"
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string evtFloodTile(int x, int y, bool flooded) const;

    /**
     * @brief Build the "evt_biome_set X Y <name>" custom biome line.
     *
     * @param x tile horizontal coordinate
     * @param y tile vertical coordinate
     * @param biome the tile's biome
     * @return std::string the formatted line
     */
    [[nodiscard]] std::string evtBiomeSet(int x, int y,
        game::Biome biome) const;

private:
    const game::World &_world;
};

} // namespace zappy::server::gui

#endif /* !SERVER_GUI_GUILINEBUILDER_HPP_ */
