/*
** EPITECH PROJECT, 2026
** BoardData.cpp
** File description:
** BoardData
*/

#include "gui/board_data/BoardData.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace zappy::gui::board_data {

BoardData::BoardData(const ipc::ServerInfo &server_info)
    : _server_info(server_info)
{
}

void BoardData::apply(const net::ServerEvent &ev)
{
    using K = net::EventKind;
    switch (ev.kind) {
        case K::EBO: on_ebo(ev); break;
        case K::MSZ: on_msz(ev); break;
        case K::SGT: case K::SST: on_sgt(ev); break;
        case K::TNA: on_tna(ev); break;
        case K::BCT: on_bct(ev); break;
        case K::PNW: on_pnw(ev); break;
        case K::PPO: on_ppo(ev); break;
        case K::PLV: on_plv(ev); break;
        case K::PIN: on_pin(ev); break;
        case K::PDI: on_pdi(ev); break;
        case K::ENW: on_enw(ev); break;
        case K::EDI: on_edi(ev); break;
        case K::SEG: on_seg(ev); break;
        case K::PBC: on_pbc(ev); break;
        case K::PIC: on_pic(ev); break;
        case K::PIE: on_pie(ev); break;
        case K::PEX: on_pex(ev); break;
        case K::PFK: on_pfk(ev); break;
        case K::OK: on_admin_ok(ev); break;
        case K::KO: on_admin_fail("ko"); break;
        case K::SBP: on_sbp(); break;
        case K::SUC: on_suc(); break;
        case K::ADM_FLAG_LIST: on_adm_flag_list(ev); break;
        case K::PPF: on_ppf(ev); break;
        case K::EVT_BIOME_SET: on_evt_biome_set(ev); break;
        case K::EVT_STORM_START:
        case K::EVT_STORM_TICK: on_storm_set(ev); break;
        case K::EVT_STORM_END: on_storm_end(); break;
        case K::EVT_METEOR_IMPACT: on_meteor_impact(ev); break;
        case K::EVT_METEOR_END: break;
        case K::EVT_FLOOD_TILE: on_evt_flood_tile(ev); break;
        default: break;
    }
}

std::vector<std::string> BoardData::take_outbox()
{
    return std::exchange(_outbox, {});
}

const TileData &BoardData::get_tile_value(std::size_t x, std::size_t y) const
{
    if (static_cast<int>(x) >= _width || static_cast<int>(y) >= _height)
        throw std::out_of_range("get_tile_value() out of bounds");
    return _tiles[y * static_cast<std::size_t>(_width) + x];
}

void BoardData::resize_tiles()
{
    _tiles.assign(static_cast<std::size_t>(_width) * _height, TileData{});
    for (int y = 0; y < _height; ++y)
        for (int x = 0; x < _width; ++x)
            _tiles[idx(x, y)] = TileData{x, y, {}};
}

void BoardData::push_player_event(GraphicalEventKind kind, int player_id,
    std::string text)
{
    auto it = _players.find(player_id);
    GraphicalEvent ev;
    ev.kind = kind;
    ev.player_id = player_id;
    ev.text = std::move(text);
    if (it != _players.end()) {
        ev.x = it->second.x;
        ev.y = it->second.y;
    }
    _events[player_id] = std::move(ev);
}

void BoardData::on_ebo(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    auto it = _eggs.find(ev.ints[0]);
    if (it != _eggs.end())
        it->second.alive = false;
}

void BoardData::on_msz(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 2)
        return;
    if (ev.ints[0] == _width && ev.ints[1] == _height)
        return;
    _width = ev.ints[0];
    _height = ev.ints[1];
    resize_tiles();
}

void BoardData::on_sgt(const net::ServerEvent &ev)
{
    if (!ev.ints.empty())
        _time_unit = ev.ints[0];
}

void BoardData::on_tna(const net::ServerEvent &ev)
{
    if (!ev.text.empty())
        _teams.push_back(ev.text);
}

void BoardData::on_bct(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 9)
        return;
    const int x = ev.ints[0];
    const int y = ev.ints[1];
    if (x < 0 || y < 0 || x >= _width || y >= _height)
        return;
    TileData &t = _tiles[idx(x, y)];
    t.x = x;
    t.y = y;
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        t.res[i] = ev.ints[2 + i];
}

void BoardData::on_pnw(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 5)
        return;
    PlayerData p;
    p.id = ev.ints[0];
    p.x = ev.ints[1];
    p.y = ev.ints[2];
    p.orient = static_cast<Orientation>(ev.ints[3]);
    p.level = ev.ints[4];
    p.team = ev.text;
    _players[p.id] = std::move(p);
}

void BoardData::on_ppo(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 4)
        return;
    auto it = _players.find(ev.ints[0]);
    if (it == _players.end())
        return;
    it->second.x = ev.ints[1];
    it->second.y = ev.ints[2];
    it->second.orient = static_cast<Orientation>(ev.ints[3]);
}

void BoardData::on_plv(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 2)
        return;
    auto it = _players.find(ev.ints[0]);
    if (it == _players.end())
        return;
    if (ev.ints[1] > it->second.level)
        _pending_levelups.push_back(ev.ints[0]);
    it->second.level = ev.ints[1];
}

void BoardData::on_pin(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 10)
        return;
    auto it = _players.find(ev.ints[0]);
    if (it == _players.end())
        return;
    it->second.x = ev.ints[1];
    it->second.y = ev.ints[2];
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        it->second.inventory[i] = ev.ints[3 + i];
}

void BoardData::on_pdi(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    auto it = _players.find(ev.ints[0]);
    if (it != _players.end())
        it->second.alive = false;
}

void BoardData::on_enw(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 4)
        return;
    EggData e;
    e.id = ev.ints[0];
    e.player_id = ev.ints[1];
    e.x = ev.ints[2];
    e.y = ev.ints[3];
    e.alive = true;
    _eggs[e.id] = e;
}

void BoardData::on_edi(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    auto it = _eggs.find(ev.ints[0]);
    if (it != _eggs.end())
        it->second.alive = false;
}

void BoardData::on_seg(const net::ServerEvent &ev)
{
    _ended = true;
    _winner = ev.text;
}

void BoardData::on_pbc(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    push_player_event(GraphicalEventKind::BROADCAST, ev.ints[0], ev.text);
}

void BoardData::on_pic(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 3)
        return;
    GraphicalEvent gev;
    gev.kind = GraphicalEventKind::INCANTATION_START;
    gev.x = ev.ints[0];
    gev.y = ev.ints[1];
    gev.level = ev.ints[2];
    gev.success = false;
    gev.alive = true;
    _events[gev.player_id] = std::move(gev);
}

void BoardData::on_pie(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 3)
        return;
    GraphicalEvent gev;
    gev.kind = GraphicalEventKind::INCANTATION_END;
    gev.x = ev.ints[0];
    gev.y = ev.ints[1];
    gev.success = ev.ints[2] != 0;
    gev.alive = false;
    _events[gev.player_id] = std::move(gev);
}

void BoardData::on_pex(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    push_player_event(GraphicalEventKind::EJECT, ev.ints[0], {});
}

void BoardData::on_pfk(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    push_player_event(GraphicalEventKind::FORK, ev.ints[0], {});
}

void BoardData::set_admin_status(std::string code)
{
    _admin_status = std::move(code);
    ++_admin_status_seq;
}

void BoardData::on_admin_ok(const net::ServerEvent &)
{
    set_admin_status("ok");
    if (!_admin_pending)
        return;
    _admin_pending = false;
    _is_admin = true;
    _outbox.emplace_back("adm_flag_list\n");
}

void BoardData::on_admin_fail(std::string code)
{
    set_admin_status(std::move(code));
    _admin_pending = false;
}

void BoardData::on_adm_flag_list(const net::ServerEvent &ev)
{
    const std::size_t sep = ev.text.find(' ');
    if (sep == std::string::npos)
        return;
    const std::string name = ev.text.substr(0, sep);
    const std::string state = ev.text.substr(sep + 1);
    _feature_flags[name] = (state == "on");
}

void BoardData::on_suc()
{
    if (_ppf_pending) {
        _ppf_pending = false;
        _profile_disabled = true;
        return;
    }
    on_admin_fail("suc");
}

void BoardData::on_sbp()
{
    if (_ppf_pending) {
        _ppf_pending = false;
        return;
    }
    on_admin_fail("sbp");
}

void BoardData::set_selected_player(int id)
{
    _selected_player_id = id;
    _profile_disabled = false;
    _ppf_pending = true;
    _outbox.emplace_back("ppf #" + std::to_string(id) + "\n");
}

void BoardData::select_player_local(int id) noexcept
{
    _selected_player_id = id;
    _ppf_pending = false;
    _profile_disabled = false;
}

void BoardData::clear_selection() noexcept
{
    _selected_player_id = -1;
    _ppf_pending = false;
    _profile_disabled = false;
}

bool BoardData::fill_from_ppf(PlayerData &p, const std::string &text)
{
    std::istringstream iss(text);
    std::string team;
    int x = 0;
    int y = 0;
    int o = 0;
    int l = 0;
    Resources inv{};

    if (!(iss >> team >> x >> y >> o >> l))
        return false;
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i)
        if (!(iss >> inv[i]))
            return false;
    p.team = team;
    p.x = x;
    p.y = y;
    p.level = l;
    p.orient = static_cast<Orientation>(o);
    p.inventory = inv;
    return true;
}

void BoardData::on_ppf(const net::ServerEvent &ev)
{
    if (ev.ints.empty())
        return;
    auto it = _players.find(ev.ints[0]);
    if (it != _players.end() && fill_from_ppf(it->second, ev.text)) {
        _ppf_pending = false;
        _profile_disabled = false;
    }
}

void BoardData::on_evt_biome_set(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 2)
        return;
    const int x = ev.ints[0];
    const int y = ev.ints[1];
    if (x < 0 || y < 0 || x >= _width || y >= _height)
        return;
    _tiles[idx(x, y)].biome = biome_from_name(ev.text);
}

void BoardData::on_storm_set(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 4)
        return;
    _storm = StormZone(ev.ints[0], ev.ints[1], ev.ints[2],
        static_cast<Orientation>(ev.ints[3]));
}

void BoardData::on_storm_end()
{
    _storm.reset();
}

void BoardData::on_meteor_impact(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 3)
        return;
    _pending_meteors.emplace_back(MeteorImpact(ev.ints[0], ev.ints[1], ev.ints[2]));
}

std::vector<MeteorImpact> BoardData::take_pending_meteors()
{
    return std::exchange(_pending_meteors, {});
}

std::vector<int> BoardData::take_pending_levelups()
{
    return std::exchange(_pending_levelups, {});
}

std::unordered_map<int, GraphicalEvent> BoardData::take_events()
{
    return std::exchange(_events, {});
}

void BoardData::on_evt_flood_tile(const net::ServerEvent &ev)
{
    if (ev.ints.size() < 2)
        return;
    const int x = ev.ints[0];
    const int y = ev.ints[1];
    if (x < 0 || y < 0 || x >= _width || y >= _height)
        return;
    _tiles[idx(x, y)].flooded = (ev.text == "on");
}

} // namespace zappy::gui::board_data
