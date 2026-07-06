/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** ServerEventParser implementation
*/

#include "gui/net/ServerEvent.hpp"

#include <array>
#include <charconv>
#include <string>
#include <string_view>

namespace zappy::gui::net {

namespace {

struct VerbInfo {
    std::string_view name;
    EventKind kind;
    int int_count;
    bool has_tail;
};

constexpr std::array<VerbInfo, 35> VERBS = {{
    {"ebo", EventKind::EBO, 1, false},
    {"msz", EventKind::MSZ, 2, false}, {"sgt", EventKind::SGT, 1, false},
    {"sst", EventKind::SST, 1, false}, {"tna", EventKind::TNA, 0, true},
    {"bct", EventKind::BCT, 9, false}, {"pnw", EventKind::PNW, 5, true},
    {"ppo", EventKind::PPO, 4, false}, {"plv", EventKind::PLV, 2, false},
    {"pin", EventKind::PIN, 10, false}, {"pex", EventKind::PEX, 1, false},
    {"pbc", EventKind::PBC, 1, true}, {"pic", EventKind::PIC, 0, true},
    {"pie", EventKind::PIE, 3, false}, {"pfk", EventKind::PFK, 1, false},
    {"pdr", EventKind::PDR, 2, false},
    {"pgt", EventKind::PGT, 2, false}, {"pdi", EventKind::PDI, 1, false},
    {"enw", EventKind::ENW, 4, false}, {"edi", EventKind::EDI, 1, false},
    {"seg", EventKind::SEG, 0, true}, {"smg", EventKind::SMG, 0, true},
    {"suc", EventKind::SUC, 0, false}, {"sbp", EventKind::SBP, 0, false},
    {"ok", EventKind::OK, 0, false}, {"ko", EventKind::KO, 0, false},
    {"adm_flag_list", EventKind::ADM_FLAG_LIST, 0, true},
    {"ppf", EventKind::PPF, 1, true},
    {"evt_biome_set", EventKind::EVT_BIOME_SET, 2, true},
    {"evt_storm_start", EventKind::EVT_STORM_START, 4, false},
    {"evt_storm_tick", EventKind::EVT_STORM_TICK, 4, false},
    {"evt_storm_end", EventKind::EVT_STORM_END, 0, false},
    {"evt_meteor_impact", EventKind::EVT_METEOR_IMPACT, 3, false},
    {"evt_meteor_end", EventKind::EVT_METEOR_END, 0, false},
    {"evt_flood_tile", EventKind::EVT_FLOOD_TILE, 2, true}
}};

std::string_view trim(std::string_view s) noexcept
{
    const auto a = s.find_first_not_of(" \t\r\n");
    const auto b = s.find_last_not_of(" \t\r\n");
    return (a == std::string_view::npos)
        ? std::string_view{} : s.substr(a, b - a + 1);
}

std::vector<std::string_view> tokenize(std::string_view s)
{
    std::vector<std::string_view> out;
    std::size_t i = s.find_first_not_of(" \t\r\n");
    while (i != std::string_view::npos) {
        const std::size_t j = s.find_first_of(" \t\r\n", i);
        out.push_back(j == std::string_view::npos
            ? s.substr(i) : s.substr(i, j - i));
        i = (j == std::string_view::npos)
            ? std::string_view::npos : s.find_first_not_of(" \t\r\n", j);
    }
    return out;
}

/*
bool to_int(std::string_view tok, int &out) noexcept
{
    if (!tok.empty() && tok.front() == '#')
        tok.remove_prefix(1);
    const auto *e = tok.data() + tok.size();
    return std::from_chars(tok.data(), e, out).ptr == e;
}
*/

static bool to_int(std::string_view tok, int &out) noexcept
{
    std::from_chars_result result;

    if (!tok.empty() && tok.front() == '#')
        tok.remove_prefix(1);
    const auto *e = tok.data() + tok.size();
    result = std::from_chars(tok.data(), e, out);
    if (result.ec == std::errc::invalid_argument)
        out = 0;
    if (result.ec == std::errc::result_out_of_range)
        out = 0;
    return result.ptr == e;
}

const VerbInfo *find_verb(std::string_view verb) noexcept
{
    for (const auto &v : VERBS)
        if (v.name == verb)
            return &v;
    return nullptr;
}

std::string tail_after(std::string_view rest,
    const std::vector<std::string_view> &tokens, std::size_t skipped)
{
    if (skipped >= tokens.size())
        return {};
    const std::size_t off = static_cast<std::size_t>(
        tokens[skipped].data() - rest.data());
    return std::string(trim(rest.substr(off)));
}

void fill_all_ints(ServerEvent &ev,
    const std::vector<std::string_view> &tokens)
{
    for (auto t : tokens) {
        int v;
        if (to_int(t, v))
            ev.ints.push_back(v);
    }
}

bool fill_leading_ints(ServerEvent &ev, const VerbInfo &info,
    const std::vector<std::string_view> &tokens, std::size_t &consumed)
{
    for (; consumed < static_cast<std::size_t>(info.int_count)
         && consumed < tokens.size(); ++consumed) {
        int v = 0;
        if (!to_int(tokens[consumed], v))
            return false;
        ev.ints.push_back(v);
    }
    return true;
}

void fill_from_info(ServerEvent &ev, const VerbInfo &info,
    std::string_view rest)
{
    const auto tokens = tokenize(rest);
    if (info.kind == EventKind::PIC) {
        fill_all_ints(ev, tokens);
        return;
    }
    std::size_t consumed = 0;
    if (!fill_leading_ints(ev, info, tokens, consumed)) {
        ev.kind = EventKind::UNKNOWN;
        ev.ints.clear();
        return;
    }
    if (info.has_tail)
        ev.text = tail_after(rest, tokens, consumed);
}

} // namespace

ServerEvent ServerEventParser::parse(std::string_view line)
{
    ServerEvent ev;
    const std::string_view trimmed = trim(line);
    if (trimmed.empty())
        return ev;
    const std::size_t sep = trimmed.find_first_of(" \t\r\n");
    const std::string_view verb = trimmed.substr(0, sep);
    const std::string_view rest = (sep == std::string_view::npos)
        ? std::string_view{} : trim(trimmed.substr(sep));
    const VerbInfo *info = find_verb(verb);
    if (info == nullptr)
        return ev;
    ev.kind = info->kind;
    fill_from_info(ev, *info, rest);
    return ev;
}

} // namespace zappy::gui::net
