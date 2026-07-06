/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Scene implementation
*/

#include "gui/render/Scene.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>
#include <algorithm>

namespace zappy::gui::render {

namespace {

constexpr float TILE_UNIT = 6.0f;
constexpr int CHUNK_SIZE = 4;
constexpr int MAX_RENDER_CHUNKS = 32;
constexpr float ROBOT_HALF = 2.5f;
constexpr float ROBOT_LOW = -1.0f;
constexpr float ROBOT_HIGH = 6.0f;
constexpr double METEOR_DURATION = 1.5;
constexpr double LEVELUP_DURATION = 1.5;
constexpr double EJECT_DURATION = 0.6;
constexpr double WALK_DURATION = 0.3;
constexpr double DEATH_DURATION = 0.3;

constexpr std::array<int, board_data::BIOME_COUNT> BIOME_MODELS = {
    0, 1, 2, 3, 4, 4
};

constexpr int OCEAN_MODEL = 5;

Vector3 dir_vec(board_data::Orientation dir)
{
    switch (dir) {
        case board_data::Orientation::NORTH: return {0.0f, 0.0f, 1.0f};
        case board_data::Orientation::EAST: return {1.0f, 0.0f, 0.0f};
        case board_data::Orientation::SOUTH: return {0.0f, 0.0f, -1.0f};
        default: return {-1.0f, 0.0f, 0.0f};
    }
}

} // namespace

Vector3 Scene::world_pos(int x, int y) const
{
    const float wave = _world ? _world->get_wave_pos(x, y) : 0.0f;

    return {static_cast<float>(x) * TILE_UNIT, wave,
        static_cast<float>(y) * TILE_UNIT};
}

BoundingBox Scene::player_box(int x, int y) const
{
    const Vector3 p = world_pos(x, y);

    return {{p.x - ROBOT_HALF, ROBOT_LOW, p.z - ROBOT_HALF},
        {p.x + ROBOT_HALF, ROBOT_HIGH, p.z + ROBOT_HALF}};
}

void Scene::set_render_distance(int chunks) noexcept
{
    _render_chunks = std::clamp(chunks, 0, MAX_RENDER_CHUNKS);
}

void Scene::trigger_eject(int player_id)
{
    const auto it = _slots.find(player_id);

    if (it != _slots.end())
        it->second.punch_until = GetTime() + EJECT_DURATION;
}

std::pair<int, int> Scene::focus_tile(const Camera &cam) const
{
    const Vector3 pos = cam.position;
    const Vector3 dir = {cam.target.x - pos.x, cam.target.y - pos.y,
        cam.target.z - pos.z};
    float fx = pos.x;
    float fz = pos.z;

    if (dir.y < 0.0f) {
        const float t = -pos.y / dir.y;
        fx = pos.x + dir.x * t;
        fz = pos.z + dir.z * t;
    }
    return {static_cast<int>(std::lround(fx / TILE_UNIT)),
        static_cast<int>(std::lround(fz / TILE_UNIT))};
}

bool Scene::in_range(int x, int y) const noexcept
{
    if (_range_tiles <= 0)
        return true;
    return std::abs(x - _focus_x) <= _range_tiles
        && std::abs(y - _focus_y) <= _range_tiles;
}

void Scene::set_shader(Shader shader)
{
    _shader = shader;
    _has_shader = true;
    if (_world)
        _world->set_shader(shader);
    for (auto &kv : _slots)
        kv.second.robot->set_shader(shader);
    for (auto &kv : _levelups)
        kv.second.fx->set_shader(shader);
}

void Scene::drain_meteors(board_data::BoardData &board)
{
    for (const auto &m : board.take_pending_meteors())
        _meteors.push_back({m.x, m.y, m.radius, GetTime()});
}

void Scene::drain_levelups(board_data::BoardData &board)
{
    for (int id : board.take_pending_levelups()) {
        const auto pit = board.players().find(id);
        if (pit == board.players().end())
            continue;
        auto [it, inserted] = _levelups.try_emplace(id);
        LevelUpFx &slot = it->second;
        if (inserted)
            slot.fx = std::make_unique<::LevelUp>();
        if (inserted && _has_shader)
            slot.fx->set_shader(_shader);
        slot.start_time = GetTime();
        slot.x = pit->second.x;
        slot.y = pit->second.y;
    }
}

void Scene::draw_levelups()
{
    const double now = GetTime();

    for (auto &kv : _levelups) {
        kv.second.fx->set_pos(world_pos(kv.second.x, kv.second.y));
        kv.second.fx->draw();
    }
    std::erase_if(_levelups, [now](const auto &kv) {
        return now - kv.second.start_time >= LEVELUP_DURATION;
    });
}

void Scene::sync_items(const board_data::BoardData &board)
{
    for (int x = 0; x < _width; ++x)
        for (int y = 0; y < _height; ++y)
            sync_item(board.get_tile_value(x, y),
                std::make_pair(static_cast<std::size_t>(x),
                    static_cast<std::size_t>(y)));
}

void Scene::sync(board_data::BoardData &board)
{
    ensure_world(board);
    apply_biome_models(board);
    drain_meteors(board);
    drain_levelups(board);
    for (const auto &kv : board.players())
        sync_player(kv.first, kv.second);
    for (const auto &egg : board.eggs()) {
        sync_egg(egg.first, egg.second);
    }
    for (const auto &event : board.events()) {
        sync_incant(event.first, event.second);
    }
    sync_items(board);
}

int Scene::tile_model_index(const board_data::TileData &tile) const
{
    if (_show_flood && tile.flooded)
        return OCEAN_MODEL;
    if (_show_biomes)
        return BIOME_MODELS[static_cast<std::size_t>(tile.biome)];
    return 0;
}

void Scene::apply_biome_models(const board_data::BoardData &board)
{
    if (!_world)
        return;
    for (const auto &tile : board.tiles())
        _world->set_tile_model(tile.x, tile.y, tile_model_index(tile));
}

void Scene::ensure_world(const board_data::BoardData &board)
{
    int w = board.width();
    int h = board.height();
    if (w <= 0 || h <= 0)
        return;
    if (_world && w == _width && h == _height)
        return;
    _world = std::make_unique<::world>(w, h);
    if (_has_shader)
        _world->set_shader(_shader);
    _width = w;
    _height = h;
}

void Scene::sync_player(int id, const board_data::PlayerData &player)
{
    auto [it, inserted] = _slots.try_emplace(id);
    Vector3 player_pos = world_pos(player.x, player.y);

    Slot &slot = it->second;
    if (inserted) {
        slot.robot = std::make_unique<::trantor>(player, player_pos);
    } else {
        // Animate a slide only for a true adjacent step (no wrap/teleport).
        const int dx = std::abs(player.x - slot.x);
        const int dy = std::abs(player.y - slot.y);
        if (dx + dy == 1) {
            slot.move_from = world_pos(slot.x, slot.y);
            slot.move_to = player_pos;
            slot.move_start = GetTime();
            slot.move_until = slot.move_start + WALK_DURATION;
        }
        slot.robot->sync(player, player_pos);
    }
    slot.x = player.x;
    slot.y = player.y;
    rotate_to(slot, static_cast<int>(player.orient));
<<<<<<< HEAD
    if (!player.alive || slot.dead) {
=======
    slot.robot->_mod->Update_anim();
    if (!player.alive && !slot.dead) {
        if (slot.death_bool == false) {
            slot.death_bool = true;
            slot.death_start = GetTime();
        }
        slot.death_until = slot.death_start + DEATH_DURATION;
>>>>>>> 6531239731f8ad65eb87f2a005a276311667e025
        slot.robot->death();
    }
}

void Scene::sync_item(const board_data::TileData &tile,
    std::pair<std::size_t, std::size_t> coords)
{
    auto [it, inserted] = _items.try_emplace(coords);
    std::unique_ptr<Item> &item_ptr = it->second;

    if (inserted) {
        item_ptr = std::make_unique<::Item>(tile.res,
            world_pos(static_cast<int>(coords.first),
                static_cast<int>(coords.second)));
    } else {
        item_ptr->sync(tile.res);
    }
    item_ptr->set_pos(world_pos(coords.first, coords.second));
}

void Scene::sync_egg(int id, const board_data::EggData &egg)
{
    auto [it, inserted] = _eggs.try_emplace(id);
    EggSlot &slot = it->second;

    if (inserted) {
        slot.egg = std::make_unique<::Egg>();
    }
    slot.x = egg.x;
    slot.y = egg.y;
    slot.egg->set_pos(world_pos(egg.x, egg.y));
    if (!egg.alive && !slot.dead)
        slot.dead = true;
}

void Scene::rotate_to(Slot &slot, int target)
{
    int diff = ((target - slot.orientation) % 4 + 4) % 4;
    if (diff == 0)
        return;
    if (diff == 3) {
        slot.robot->Turn_left();
    } else {
        for (int i = 0; i < diff; ++i)
            slot.robot->Turn_right();
    }
    slot.orientation = target;
}

void Scene::draw_storm_arrow(Vector3 center, board_data::Orientation dir)
{
    const Vector3 d = dir_vec(dir);
    const Vector3 base = {center.x, 1.0f, center.z};
    const Vector3 mid = {base.x + d.x * 4.0f, base.y, base.z + d.z * 4.0f};
    const Vector3 tip = {base.x + d.x * 6.0f, base.y, base.z + d.z * 6.0f};

    DrawCylinderEx(base, mid, 0.3f, 0.3f, 8, YELLOW);
    DrawCylinderEx(mid, tip, 0.9f, 0.0f, 12, ORANGE);
}

void render::Scene::sync_incant(int id, const board_data::GraphicalEvent &event)
{
    if (event.kind != board_data::GraphicalEventKind::INCANTATION_START &&
        event.kind != board_data::GraphicalEventKind::INCANTATION_END)
        return;
    auto [it, inserted] = this->_incantation.try_emplace(id);
    Incant &slot = it->second;
    if (inserted) {
        slot.incant = std::make_unique<::Incantation>();
    }
    slot.incant->set_pos(world_pos(event.x, event.y));
    if (!event.alive && !slot.dead)
        slot.dead = true;
}

void Scene::draw_storm(const board_data::StormZone &storm)
{
    const Vector3 c = world_pos(storm.center_x, storm.center_y);
    const float r = static_cast<float>(storm.radius) * TILE_UNIT;

    DrawCylinder({c.x, 0.2f, c.z}, r, r, 0.05f, 32, {80, 120, 200, 90});
    draw_storm_arrow(c, storm.direction);
}

void Scene::draw_meteor(const MeteorView &view)
{
    const double elapsed = GetTime() - view.start_time;
    const float ratio = static_cast<float>(elapsed / METEOR_DURATION);

    if (ratio >= 1.0f)
        return;
    const Vector3 base = world_pos(view.x, view.y);
    const float full = static_cast<float>(view.radius) * TILE_UNIT;
    const float rad = full * (0.4f + ratio * 0.6f);
    const auto a = static_cast<unsigned char>(255.0f * (1.0f - ratio));
    const auto ga = static_cast<unsigned char>(100.0f * (1.0f - ratio));

    DrawSphere({base.x, full * 0.5f, base.z}, rad, {255, 100, 0, a});
    DrawCylinder({base.x, 0.15f, base.z}, full, full, 0.05f, 32,
        {255, 200, 0, ga});
}

void Scene::draw_meteors()
{
    const double now = GetTime();

    for (const auto &m : _meteors)
        draw_meteor(m);
    std::erase_if(_meteors, [now](const MeteorView &m) {
        return now - m.start_time >= METEOR_DURATION;
    });
}

void Scene::draw_robot(Slot &slot)
{
    const double now = GetTime();

    if (now < slot.punch_until) {
        slot.robot->eject_player();
    } else if (now < slot.move_until) {
        const float span = static_cast<float>(slot.move_until
            - slot.move_start);
        const float t = span > 0.0f ? std::clamp(static_cast<float>(
            now - slot.move_start) / span, 0.0f, 1.0f) : 1.0f;
        const Vector3 &a = slot.move_from;
        const Vector3 &b = slot.move_to;
        slot.robot->set_pos({a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t});
        slot.robot->walk();
    } else if (now > slot.death_until && slot.death_bool == true) {
        slot.dead = true;
    }
    slot.robot->draw();
}

void Scene::draw(const board_data::BoardData &board, const Camera &cam)
{
    const auto [cx, cy] = focus_tile(cam);

    if (_render_chunks > 0) {
        _focus_x = (cx / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE / 2;
        _focus_y = (cy / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE / 2;
        _range_tiles = _render_chunks * CHUNK_SIZE;
    } else {
        _range_tiles = 0;
    }
    if (_world)
        _world->draw_world(_focus_x, _focus_y, _range_tiles);
    trantor::_mod->Update_anim();
    for (auto &kv : _slots) {
        if (in_range(kv.second.x, kv.second.y))
            draw_robot(kv.second);
    }
    for (auto &eg : _eggs) {
        if (!eg.second.dead && in_range(eg.second.x, eg.second.y))
            eg.second.egg->draw();
    }
    for (auto &ic : _incantation) {
        if (!ic.second.dead) {
            ic.second.incant->draw();
        }
    }
    for (auto &kv : _items) {
        if (in_range(static_cast<int>(kv.first.first),
            static_cast<int>(kv.first.second)))
            kv.second->draw();
    }
    if (_show_storm && board.storm().has_value())
        draw_storm(*board.storm());
    draw_meteors();
    draw_levelups();
}

std::optional<int> Scene::pick_player(Vector2 screen, const Camera &cam) const
{
    const Ray ray = GetScreenToWorldRay(screen, cam);
    float best = 0.0f;
    int found = -1;

    for (const auto &kv : _slots) {
        if (kv.second.dead)
            continue;
        const RayCollision hit =
            GetRayCollisionBox(ray, player_box(kv.second.x, kv.second.y));
        if (hit.hit && (found < 0 || hit.distance < best)) {
            best = hit.distance;
            found = kv.first;
        }
    }
    if (found < 0)
        return std::nullopt;
    return found;
}

std::optional<std::pair<int, int>> Scene::pick_tile(Vector2 screen,
    const Camera &cam) const
{
    const Ray ray = GetScreenToWorldRay(screen, cam);

    if (ray.direction.y == 0.0f)
        return std::nullopt;
    const float t = -ray.position.y / ray.direction.y;
    if (t < 0.0f)
        return std::nullopt;
    const float wx = ray.position.x + ray.direction.x * t;
    const float wz = ray.position.z + ray.direction.z * t;
    const int tx = static_cast<int>(std::lround(wx / TILE_UNIT));
    const int ty = static_cast<int>(std::lround(wz / TILE_UNIT));
    if (tx < 0 || ty < 0 || tx >= _width || ty >= _height)
        return std::nullopt;
    return std::make_pair(tx, ty);
}

} // namespace zappy::gui::render
