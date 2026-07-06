/*
** EPITECH PROJECT, 2026
** HealthBarBatch.cpp
** File description:
** HealthBarBatch
*/

#include "gui/HealthBarBatch.hpp"
#include <algorithm>

HealthBarBatch::HealthBarBatch(bool start_loaded, float start_scale)
    : _scale(start_scale), _texture(), _is_loaded(start_loaded)
{
}

HealthBarBatch::~HealthBarBatch()
{
    UnloadTexture(_texture);
}

bool HealthBarBatch::isloaded()
{
    return _is_loaded;
}

static bool healthbar_distance_compare(Vector3 camera_position,
    HealthBarBatch::HealthBarInfo a, HealthBarBatch::HealthBarInfo b)
{
    return Vector3Distance(camera_position, a.position) > Vector3Distance(camera_position, b.position);
}

template<typename tPair>
struct second_t {
    typename tPair::second_type operator()( const tPair& p ) const { return p.second; }
};

template<typename tMap> 
second_t<typename tMap::value_type> second(const tMap&)
{
    return second_t<typename tMap::value_type>();
}

template< typename KeyType, typename ValueType>
static std::vector<ValueType> map_to_vector(std::map<KeyType, ValueType> map) {
    std::vector<ValueType> result;
    std::transform( map.begin(), map.end(), std::back_inserter( result ), second(map) );
    return result;
}

#define HEALTHBAR_DISTANCE_COMPARATOR(camera_position) [camera_position](HealthBarBatch::HealthBarInfo a, HealthBarBatch::HealthBarInfo b)\
{ \
    return healthbar_distance_compare(camera_position, a, b); \
}

std::vector<HealthBarBatch::HealthBarInfo> HealthBarBatch::get_order(Vector3 camera_position)
{
    auto healthbar_vector = map_to_vector(this->healthbars_info);

    std::sort(healthbar_vector.begin(), healthbar_vector.end(), HEALTHBAR_DISTANCE_COMPARATOR(camera_position));
    return healthbar_vector;
}

void HealthBarBatch::draw_health_bar_unit(HealthBarBatch::HealthBarInfo healthbar, Camera camera)
{
    Vector3 health_up = UP_DIRECTION;
    float rotation = 0.0f;
    static Rectangle texture_rect = { 0.0f, 0.0f, (float)_texture.width, (float)_texture.height };
    Vector2 size_it = { static_cast<float>(HEALTHBAR_WIDTH * healthbar.percentage), HEALTHBAR_HEIGHT };
    size_it = Vector2Scale(size_it, _scale);
    Vector2 origin_it = Vector2Scale(size_it, 0.5f);
    Vector2 offset_translation_it = Vector2Scale(UP_V2, HEALTHBAR_HEIGHT_MODIFIER * _scale);
    Vector3 healthbar_position_it = Vector3Add(healthbar.position, {offset_translation_it.x,
        offset_translation_it.y, 0});
    DrawBillboardPro(camera, _texture, texture_rect, healthbar_position_it, health_up, size_it, origin_it, rotation, RED);
}

void HealthBarBatch::draw_health_bars(Camera camera)
{
    auto order = get_order(camera.position);

    if (!_is_loaded)
        load();
    for (HealthBarBatch::HealthBarInfo healthbar : order) {
        draw_health_bar_unit(healthbar, camera);
    }
}

void HealthBarBatch::scale_health_bars(double scale)
{
    _scale = scale;
}

void HealthBarBatch::kill(int id)
{
    healthbars_info.erase(id);
}

void HealthBarBatch::sync_health_bar(int id, Vector3 pos, double percentage)
{
    auto [it, inserted] = this->healthbars_info.try_emplace(id, HealthBarBatch::HealthBarInfo(pos, percentage));
    HealthBarInfo &healthbar = it->second;

    if (!inserted)
        healthbar.set(pos, percentage);
}

void HealthBarBatch::load()
{
    _texture = LoadTexture(TMP_HEALTH_BAR_TEXTURE);
    _is_loaded = true;
}
