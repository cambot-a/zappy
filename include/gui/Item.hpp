/*
** EPITECH PROJECT, 2026
** Item.hpp
** File description:
** Item
*/

#ifndef ITEM_HPP_
    #define ITEM_HPP_
    #include <cstddef>
    #include <memory>
    #include "model.hpp"
    #include "gui/board_data/TileData.hpp"

    #define STONE_PATH "src/gui/assets/crystals.glb"
    #define FOOD_PATH "src/gui/assets/octopus_sandwich.glb"


namespace {

constexpr float FOOD_SCALE = 0.03f;
constexpr float ITEM_LIFT = 0.0f;
constexpr std::size_t MINERAL_COUNT = 6;

const std::array<Color, MINERAL_COUNT> MINERAL_COLOR = {{
    GRAY, RED, PURPLE, ORANGE, YELLOW, GREEN,
}};

const std::array<float, MINERAL_COUNT> MINERAL_SCALE = {{
    0.23f, 0.26f, 0.29f, 0.20f, 0.30f, 0.36f,
}};

constexpr float R = 0.9f;
constexpr float A = R * 0.5f;
constexpr float B = R * 0.8660254f;

const std::array<Vector3, MINERAL_COUNT> CRYSTAL_OFFSET = {{
    {  R, 0.0f,  0.0f},
    {  A, 0.0f,  B},
    { -A, 0.0f,  B},
    { -R, 0.0f,  0.0f},
    { -A, 0.0f, -B},
    {  A, 0.0f, -B},
}};

// Food in the center of the mineral ring.
constexpr Vector3 FOOD_OFFSET = {0.0f, 0.0f, 0.0f};

} // namespace

enum ResourceIndex {
    R_FOOD = 0,
    R_LINEMATE = 1,
    R_DERAUMERE = 2,
    R_SIBUR = 3,
    R_MENDIANE = 4,
    R_PHIRAS = 5,
    R_THYSTAME = 6,
};

#define SATISFYING_FOOD_COUNT 15

class Item {
public:
    Item(const zappy::gui::board_data::Resources resources, Vector3 pos);
    ~Item() = default;
    void set_pos(Vector3 pos);
    void draw();
    void sync(const zappy::gui::board_data::Resources resources);
    static std::unique_ptr<model> _mod_food;
    static std::unique_ptr<model> _mod_mineral;
private:
    zappy::gui::board_data::Resources _resources;
    Vector3 _pos;
};

#endif /* !ITEM_HPP_ */
