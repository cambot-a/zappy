/*
** EPITECH PROJECT, 2026
** local-zappy
** File description:
** model
*/

#include "gui/model.hpp"
#include "gui/utils.hpp"
#include "gui/trantor.hpp"

model::model(std::string name, bool load, Vector3 scale, Vector3 pos)
    : _anim(nullptr) ,_animeCount(0),
    _current_frame(0), _anim_index(0),
    _color(WHITE), _name(name), _pos(pos),
    _base_pos(pos), loaded(load),
    _rotationAxis(UP_DIRECTION), _rotationAngle(0), _scale(scale)

{
    if (load)
        this->_model = LoadModel(_name.c_str());
}

model::~model()
{
    if (this->_anim != nullptr)
        UnloadModelAnimations(this->_anim, this->_animeCount);
    if (this->loaded)
        UnloadModel(this->_model);
}

void model::load()
{
    if (this->loaded)
        return;
    this->_model = LoadModel(_name.c_str());
    this->loaded = true;
}

bool model::isloaded()
{
    return this->loaded;
}

void model::Update_anim()
{
    if (this->_anim == nullptr || this->_animeCount == 0)
        return;
    this->_current_frame++;
}

void model::switch_index_anime()
{
    this->_current_frame = 0;
    this->_anim_index = (this->_anim_index + 1) % this->_animeCount;
}

void model::Draw_model(int current_frame)
{
    if (this->_anim != nullptr) {
        if (current_frame < 0) {
            current_frame = this->_current_frame;
            check_frame();
        } else {
            if (current_frame >= this->_anim[this->_anim_index].keyframeCount)
                current_frame = 0;
        }
        UpdateModelAnimation(this->_model, this->_anim[this->_anim_index], static_cast<float>(current_frame));
    }
    DrawModelEx(this->_model, this->_pos, _rotationAxis, _rotationAngle, _scale, this->_color);
}

Color model::get_color()
{
    return this->_color;
}

void model::change_color(Color cl)
{
    this->_color = cl;
}

void model::set_shader(Shader shader)
{
    for (int i = 0; i < this->_model.materialCount; ++i)
        this->_model.materials[i].shader = shader;
}

void model::load_anim()
{
    if (this->_anim != nullptr)
        UnloadModelAnimations(this->_anim, this->_animeCount);
    this->_anim = LoadModelAnimations(_name.c_str(), &this->_animeCount);
}

void model::move_model(Vector3 rotationAxis, float rotationAngle, Vector3 scale)
{
    _rotationAxis = rotationAxis;
    _rotationAngle = rotationAngle;
    _scale = scale;
}

void model::set_pos(Vector3 pos)
{
    this->_pos = pos;
}

Vector3 model::get_pos()
{
    return this->_pos;
}


BoundingBox model::get_bbox()
{
    return GetModelBoundingBox(this->_model);
}

void model::set_base_pos(Vector3 pos)
{
    this->_base_pos = pos;
}

Vector3 model::get_base_pos()
{
    return this->_base_pos;
}

void model::set_anim_index(int anime_index)
{
    this->_anim_index = anime_index;
}

void model::load_texture()
{
    this->_texture = LoadTexture(_name.c_str());
    this->_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
        = this->_texture;
}

void model::load_texture(std::string name)
{
    this->_texture = LoadTexture(name.c_str());
    this->_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
        = this->_texture;
}

int model::get_frame_count()
{
    return this->_anim[this->_anim_index].keyframeCount;
}

int model::get_current_frame()
{
    return this->_current_frame;
}

void model::check_frame()
{
    if (_current_frame >= this->_anim[this->_anim_index].keyframeCount)
        _current_frame = 0;
}