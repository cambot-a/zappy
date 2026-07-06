/*
** EPITECH PROJECT, 2026
** BoardData.hpp
** File description:
** BoardData
*/

#ifndef BOARDDATA_HPP_
    #define BOARDDATA_HPP_
    #include <cstddef>
    #include <optional>
    #include <string>
    #include <unordered_map>
    #include <vector>

    #include "gui/board_data/GraphicalEvent.hpp"
    #include "gui/board_data/TileData.hpp"
    #include "gui/ipc/ServerInfo.hpp"
    #include "gui/net/ServerEvent.hpp"

namespace zappy::gui::board_data {

const std::string default_team = "default_team";

class PlayerData {
public:
    PlayerData()
        : id(0), x(0), y(0), orient(Orientation::NORTH), level(1),
        team(default_team), inventory({0, 0, 0, 0, 0, 0, 0}), alive(true) {}
    int id;
    int x;
    int y;
    Orientation orient;
    int level;
    std::string team;
    Resources inventory;
    bool alive;
};

class EggData {
public:
    EggData() : id(0), player_id(-1), x(0), y(0), alive(true) {}
    int id;
    int player_id;
    int x;
    int y;
    bool alive;
};

class StormZone {
public:
    StormZone(int _center_x = 0, int _center_y = 0, int _radius = 0, Orientation _direction = Orientation::NORTH)
        : center_x(_center_x), center_y(_center_y), radius(_radius), direction(_direction) {}
    int center_x;
    int center_y;
    int radius;
    Orientation direction;
};

class MeteorImpact {
public:
    MeteorImpact(int _x = 0, int _y = 0, int _radius = 0) : x(_x), y(_y), radius(_radius) {}
    int x;
    int y;
    int radius;
};

class BoardData {
public:
    explicit BoardData(const ipc::ServerInfo &server_info);

    void apply(const net::ServerEvent &ev);

    [[nodiscard]] int width() const noexcept { return _width; }
    [[nodiscard]] int height() const noexcept { return _height; }
    [[nodiscard]] int time_unit() const noexcept { return _time_unit; }
    [[nodiscard]] const TileData &get_tile_value(std::size_t x,
        std::size_t y) const;
    [[nodiscard]] const std::vector<TileData> &tiles() const noexcept
        { return _tiles; }
    [[nodiscard]] const std::unordered_map<int, PlayerData> &players()
        const noexcept { return _players; }
    [[nodiscard]] const std::unordered_map<int, EggData> &eggs()
        const noexcept { return _eggs; }
    [[nodiscard]] const std::vector<std::string> &teams()
        const noexcept { return _teams; }
    [[nodiscard]] const std::unordered_map<int, GraphicalEvent> &events()
        const noexcept { return _events; }
    // Drain accumulated graphical events (broadcasts, ejects, ...).
    [[nodiscard]] std::unordered_map<int, GraphicalEvent>  take_events();
    [[nodiscard]] bool game_ended() const noexcept { return _ended; }
    [[nodiscard]] const std::string &winner() const noexcept { return _winner; }
    [[nodiscard]] bool is_admin() const noexcept { return _is_admin; }
    [[nodiscard]] const std::unordered_map<std::string, bool> &feature_flags()
        const noexcept { return _feature_flags; }
    [[nodiscard]] const std::string &admin_status() const noexcept
        { return _admin_status; }
    [[nodiscard]] unsigned admin_status_seq() const noexcept
        { return _admin_status_seq; }
    void mark_admin_pending() noexcept { _admin_pending = true; }
    void queue_admin_command(std::string cmd)
        { _outbox.push_back(std::move(cmd)); }
    [[nodiscard]] std::vector<std::string> take_outbox();
    [[nodiscard]] int selected_player_id() const noexcept
        { return _selected_player_id; }
    [[nodiscard]] bool profile_disabled() const noexcept
        { return _profile_disabled; }
    void set_selected_player(int id);
    void select_player_local(int id) noexcept;
    void clear_selection() noexcept;
    [[nodiscard]] const std::optional<StormZone> &storm() const noexcept
        { return _storm; }
    [[nodiscard]] std::vector<MeteorImpact> take_pending_meteors();
    [[nodiscard]] std::vector<int> take_pending_levelups();
    void arm_event(std::string name, int radius)
        { _armed_event = std::move(name); _armed_radius = radius;
          _event_armed = true; }
    void disarm_event() noexcept { _event_armed = false; }
    [[nodiscard]] bool event_armed() const noexcept { return _event_armed; }
    [[nodiscard]] const std::string &armed_event() const noexcept
        { return _armed_event; }
    [[nodiscard]] int armed_radius() const noexcept { return _armed_radius; }

private:
    void on_ebo(const net::ServerEvent &);
    void on_msz(const net::ServerEvent &);
    void on_sgt(const net::ServerEvent &);
    void on_tna(const net::ServerEvent &);
    void on_bct(const net::ServerEvent &);
    void on_pnw(const net::ServerEvent &);
    void on_ppo(const net::ServerEvent &);
    void on_plv(const net::ServerEvent &);
    void on_pin(const net::ServerEvent &);
    void on_pdi(const net::ServerEvent &);
    void on_enw(const net::ServerEvent &);
    void on_edi(const net::ServerEvent &);
    void on_seg(const net::ServerEvent &);
    void on_pbc(const net::ServerEvent &);
    void on_pic(const net::ServerEvent &);
    void on_pie(const net::ServerEvent &);
    void on_pex(const net::ServerEvent &);
    void on_pfk(const net::ServerEvent &);
    void on_admin_ok(const net::ServerEvent &);
    void on_admin_fail(std::string code);
    void on_adm_flag_list(const net::ServerEvent &);
    void on_suc();
    void on_sbp();
    void on_ppf(const net::ServerEvent &);
    void on_evt_biome_set(const net::ServerEvent &);
    void on_storm_set(const net::ServerEvent &);
    void on_storm_end();
    void on_meteor_impact(const net::ServerEvent &);
    void on_evt_flood_tile(const net::ServerEvent &);
    bool fill_from_ppf(PlayerData &p, const std::string &text);
    void set_admin_status(std::string code);

    void resize_tiles();
    void push_player_event(GraphicalEventKind kind, int player_id,
        std::string text);
    [[nodiscard]] std::size_t idx(int x, int y) const noexcept
        { return static_cast<std::size_t>(y) * _width + x; }

    ipc::ServerInfo _server_info;
    int _width = 0;
    int _height = 0;
    int _time_unit = 100;
    std::vector<TileData> _tiles;
    std::unordered_map<int, PlayerData> _players;
    std::unordered_map<int, EggData> _eggs;
    std::vector<std::string> _teams;
    std::unordered_map<int, GraphicalEvent> _events;
    bool _ended = false;
    std::string _winner;
    bool _is_admin = false;
    bool _admin_pending = false;
    std::unordered_map<std::string, bool> _feature_flags;
    std::vector<std::string> _outbox;
    std::string _admin_status;
    unsigned _admin_status_seq = 0;
    int _selected_player_id = -1;
    bool _ppf_pending = false;
    bool _profile_disabled = false;
    std::optional<StormZone> _storm;
    std::vector<MeteorImpact> _pending_meteors;
    std::vector<int> _pending_levelups;
    bool _event_armed = false;
    std::string _armed_event;
    int _armed_radius = 0;
};

} // namespace zappy::gui::board_data

#endif /* BOARDDATA_HPP_ */
