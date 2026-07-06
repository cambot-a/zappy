/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** AiDispatcher implementation
*/

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "server/ai/AiDispatcher.hpp"
#include "server/game/OrientationHelper.hpp"
#include "protocol/ai/CommandInfo.hpp"
#include "server/game/Constants.hpp"
#include "server/game/ResourceNameResolver.hpp"
#include "server/game/BroadcastDirection.hpp"

/**
 * @brief Bind the dispatcher to the world, scheduler and response sink.
 *
 * @param world game world the commands act upon
 * @param scheduler event queue timing the commands
 * @param frequency reciprocal of the time unit (the f config value)
 * @param onResponse callback delivering a response line to a player
 */
zappy::server::ai::AiDispatcher::AiDispatcher(game::World &world,
    scheduler::Scheduler &scheduler, int frequency, OnResponse onResponse)
    : _world(world), _scheduler(scheduler), _frequency(frequency),
      _onResponse(std::move(onResponse)), _lookBuilder(_world)
{}

/**
 * @brief Parse and enqueue @p line, answering "ko" immediately if malformed.
 *
 * @param playerId player issuing the command
 * @param line raw command line
 */
void zappy::server::ai::AiDispatcher::dispatch(int playerId,
    std::string_view line)
{
    const std::optional<protocol::ai::ParsedCommand> parsed =
        protocol::ai::CommandParser::parse(line);
    if (!parsed) {
        _onResponse(playerId, "ko");
        return;
    }
    PlayerState &state = _states[playerId];
    const std::size_t outstanding =
        state.queue.size() + (state.currentEventId != 0 ? 1U : 0U);
    if (outstanding >= MAX_PENDING_COMMANDS)
        return;
    state.queue.push_back(*parsed);
    if (state.currentEventId == 0)
        startNextCommand(playerId);
}

/**
 * @brief Cancel the in-flight command and drop the queue for @p playerId.
 *
 * @param playerId player to stop; no-op if unknown
 */
void zappy::server::ai::AiDispatcher::stopPlayer(int playerId) noexcept
{
    const auto it = _states.find(playerId);
    if (it == _states.end())
        return;
    if (it->second.currentEventId != 0)
        _scheduler.cancel(it->second.currentEventId);
    _states.erase(it);
}

/**
 * @brief Update the time frequency.
 *
 * @param frequency new time frequency
 */
void zappy::server::ai::AiDispatcher::setFrequency(int frequency) noexcept
{
    _frequency = frequency;
}

/**
 * @brief Resolve instantaneous commands now, schedule the first timed one.
 *
 * @param playerId player whose queue is advanced
 */
void zappy::server::ai::AiDispatcher::startNextCommand(int playerId)
{
    while (canStartNextCommand(playerId))
        processNextQueuedCommand(playerId);
}

/**
 * @brief Whether the next queued command may start now.
 *
 * @param playerId player whose queue is inspected
 * @return bool true if a command is pending and the player is not frozen
 */
bool zappy::server::ai::AiDispatcher::canStartNextCommand(int playerId) const
{
    const auto it = _states.find(playerId);
    return it != _states.end()
        && it->second.currentEventId == 0
        && !it->second.queue.empty()
        && _world.hasPlayer(playerId)
        && _world.player(playerId).state() != game::PlayerState::INCANTING;
}

/**
 * @brief Pop the front command and route it to the right starter.
 *
 * @param playerId player whose queue advances
 */
void zappy::server::ai::AiDispatcher::processNextQueuedCommand(int playerId)
{
    PlayerState &state = _states[playerId];
    const protocol::ai::ParsedCommand cmd = state.queue.front();
    state.queue.pop_front();
    if (cmd.kind == protocol::ai::CommandKind::INCANTATION)
        startIncantation(playerId);
    else
        startStandardCommand(playerId, cmd);
}

/**
 * @brief Schedule (or instantly run) a non-incantation command.
 *
 * @param playerId player issuing the command
 * @param cmd command to start
 */
void zappy::server::ai::AiDispatcher::startStandardCommand(int playerId,
    const protocol::ai::ParsedCommand &cmd)
{
    runPreHandler(playerId, cmd.kind);
    const scheduler::Duration delay =
        protocol::ai::CommandInfo::duration(cmd.kind, _frequency);
    if (delay.count() > 0)
        _states[playerId].currentEventId = _scheduler.schedule(delay,
            [this, playerId, cmd] { onCommandComplete(playerId, cmd); });
    else
        respond(playerId, cmd);
}

/**
 * @brief Validate prerequisites then launch or reject an incantation.
 *
 * @param playerId the initiating player
 */
void zappy::server::ai::AiDispatcher::startIncantation(int playerId)
{
    const std::vector<int> participants = collectParticipants(playerId);

    if (checkPrerequisites(playerId, participants))
        beginIncantationRitual(playerId, participants);
    else {
        _states[playerId].currentEventId = 0;
        _onResponse(playerId, "ko");
    }
}

/**
 * @brief Force-start an incantation on @p playerId at the GUI admin's request.
 *
 * @param playerId the player to elevate
 * @return bool true if the ritual was started, false if prerequisites fail
 */
bool zappy::server::ai::AiDispatcher::adminStartIncantation(int playerId)
{
    const std::vector<int> participants = collectParticipants(playerId);

    if (!isPlayerAlive(playerId)
        || _world.player(playerId).state() == game::PlayerState::INCANTING
        || _world.player(playerId).level() >= game::ElevationRules::maxLevel())
        return false;
    _states[playerId].forcedIncant = true;
    beginIncantationRitual(playerId, participants);
    return true;
}

/**
 * @brief Force-abort the incantation @p playerId is currently performing.
 *
 * @param playerId the incanting initiator
 * @return bool true if an incantation was stopped, false otherwise
 */
bool zappy::server::ai::AiDispatcher::adminStopIncantation(int playerId)
{
    const auto it = _states.find(playerId);

    if (!_world.hasPlayer(playerId)
        || _world.player(playerId).state() != game::PlayerState::INCANTING
        || it == _states.end())
        return false;
    if (it->second.currentEventId != 0)
        _scheduler.cancel(it->second.currentEventId);
    const std::vector<int> participants = frozenParticipantsOnTile(playerId);
    applyFailure(playerId, participants);
    resumeParticipantQueues(participants);
    return true;
}

/**
 * @brief Ids of every player frozen (INCANTING) on @p playerId's tile.
 *
 * @param playerId the incanting initiator
 * @return std::vector<int> the frozen participant ids
 */
std::vector<int> zappy::server::ai::AiDispatcher::frozenParticipantsOnTile(
    int playerId) const
{
    std::vector<int> result;

    if (!_world.hasPlayer(playerId))
        return result;
    for (int id : _world.tileAt(_world.player(playerId).position()).playerIds())
        if (_world.hasPlayer(id)
            && _world.player(id).state() == game::PlayerState::INCANTING)
            result.push_back(id);
    return result;
}

/**
 * @brief Ids of every ALIVE player standing on the initiator's tile.
 *
 * @param playerId the initiating player
 * @return std::vector<int> the participant ids (includes the initiator)
 */
std::vector<int> zappy::server::ai::AiDispatcher::collectParticipants(
    int playerId) const
{
    std::vector<int> result;
    const game::Position pos = _world.player(playerId).position();
    const int level = _world.player(playerId).level();

    for (int id : _world.tileAt(pos).playerIds())
        if (isPlayerAlive(id) && _world.player(id).level() <= level)
            result.push_back(id);
    return result;
}

/**
 * @brief Whether @p playerId may elevate with @p participants right now.
 *
 * @param playerId the initiating player
 * @param participants alive players on the tile
 * @return bool true if level, headcount and stones all qualify
 */
bool zappy::server::ai::AiDispatcher::checkPrerequisites(int playerId,
    const std::vector<int> &participants) const
{
    const int level = _world.player(playerId).level();
    const game::ElevationRule &rule = game::ElevationRules::forLevel(level);
    const game::Position pos = _world.player(playerId).position();

    return level < game::ElevationRules::maxLevel()
        && static_cast<int>(participants.size()) >= rule.playersRequired
        && tileHasStones(pos, rule);
}

/**
 * @brief Whether the tile holds every stone @p rule demands.
 *
 * @param pos the tile position
 * @param rule the elevation rule to satisfy
 * @return bool true if all stones are present in sufficient quantity
 */
bool zappy::server::ai::AiDispatcher::tileHasStones(game::Position pos,
    const game::ElevationRule &rule) const
{
    bool ok = true;

    for (std::size_t i = 0; i < game::RESOURCE_COUNT; ++i)
        if (_world.tileAt(pos).resource(static_cast<game::ResourceType>(i))
            < rule.stonesRequired[i])
            ok = false;
    return ok;
}

/**
 * @brief Whether the player exists and is not DEAD.
 *
 * @param playerId player to test
 * @return bool true if alive or incanting
 */
bool zappy::server::ai::AiDispatcher::isPlayerAlive(int playerId) const
{
    return _world.hasPlayer(playerId)
        && _world.player(playerId).state() != game::PlayerState::DEAD;
}

/**
 * @brief Freeze participants, announce the ritual and schedule its end.
 *
 * @param playerId the initiating player
 * @param participants alive players on the tile
 */
void zappy::server::ai::AiDispatcher::beginIncantationRitual(int playerId,
    const std::vector<int> &participants)
{
    const int level = _world.player(playerId).level();

    for (int id : participants)
        _world.freezePlayer(id);
    for (int id : participants)
        _onResponse(id, "Elevation underway");
    _world.notifyIncantationStarted(playerId, level, participants);
    const scheduler::Duration delay = protocol::ai::CommandInfo::duration(
        protocol::ai::CommandKind::INCANTATION, _frequency);
    _states[playerId].currentEventId = _scheduler.schedule(delay,
        [this, playerId, participants] {
            finalizeIncantation(playerId, participants);
        });
}

/**
 * @brief Re-check prerequisites and apply success or failure.
 *
 * @param initiatorId the initiating player
 * @param participants the participants captured at ritual start
 */
void zappy::server::ai::AiDispatcher::finalizeIncantation(int initiatorId,
    std::vector<int> participants)
{
    std::vector<int> alive;

    const bool forced = _states[initiatorId].forcedIncant;

    std::copy_if(participants.begin(), participants.end(),
        std::back_inserter(alive),
        [this](int id) { return isPlayerAlive(id); });
    if (isPlayerAlive(initiatorId)
        && (forced || checkPrerequisites(initiatorId, alive)))
        applySuccess(initiatorId, alive,
            _world.player(initiatorId).level() + 1);
    else
        applyFailure(initiatorId, alive);
    resumeParticipantQueues(alive);
}

/**
 * @brief Elevate every alive participant and consume the tile's stones.
 *
 * @param initiatorId the initiating player
 * @param alive participants still alive at finalization
 * @param newLevel the resulting level
 */
void zappy::server::ai::AiDispatcher::applySuccess(int initiatorId,
    const std::vector<int> &alive, int newLevel)
{
    const game::Position tilePos = _world.player(initiatorId).position();
    const std::string line = "Current level: " + std::to_string(newLevel);

    consumeStones(tilePos, game::ElevationRules::forLevel(newLevel - 1));
    for (int id : alive)
        _world.setPlayerLevel(id, newLevel);
    sendToAllExceptInitiator(alive, initiatorId, line);
    _world.notifyIncantationEnded(initiatorId, true, newLevel);
    unfreezeAll(alive);
    _states[initiatorId].currentEventId = 0;
    _states[initiatorId].forcedIncant = false;
    _onResponse(initiatorId, line);
}

/**
 * @brief Reject the incantation: answer "ko" and unfreeze participants.
 *
 * @param initiatorId the initiating player
 * @param alive participants still alive at finalization
 */
void zappy::server::ai::AiDispatcher::applyFailure(int initiatorId,
    const std::vector<int> &alive)
{
    const bool initiatorAlive =
        std::find(alive.begin(), alive.end(), initiatorId) != alive.end();

    sendToAllExceptInitiator(alive, initiatorId, "ko");
    _world.notifyIncantationEnded(initiatorId, false, 0);
    unfreezeAll(alive);
    _states[initiatorId].currentEventId = 0;
    _states[initiatorId].forcedIncant = false;
    if (initiatorAlive)
        _onResponse(initiatorId, "ko");
}

/**
 * @brief Remove the stones @p rule demands from @p tilePos.
 *
 * @param tilePos the tile to deduct from
 * @param rule the elevation rule that was satisfied
 */
void zappy::server::ai::AiDispatcher::consumeStones(game::Position tilePos,
    const game::ElevationRule &rule)
{
    for (std::size_t i = 0; i < game::RESOURCE_COUNT; ++i) {
        if (rule.stonesRequired[i] <= 0)
            continue;
        const auto type = static_cast<game::ResourceType>(i);
        const int current = _world.tileAt(tilePos).resource(type);
        _world.setTileResource(tilePos, type,
            std::max(0, current - rule.stonesRequired[i]));
    }
}

/**
 * @brief Unfreeze every listed participant.
 *
 * @param participants players to unfreeze
 */
void zappy::server::ai::AiDispatcher::unfreezeAll(
    const std::vector<int> &participants)
{
    for (int id : participants)
        _world.unfreezePlayer(id);
}

/**
 * @brief Resume each participant's queue after the ritual.
 *
 * @param participants players whose queues advance
 */
void zappy::server::ai::AiDispatcher::resumeParticipantQueues(
    const std::vector<int> &participants)
{
    for (int id : participants)
        startNextCommand(id);
}

/**
 * @brief Send @p response to every participant except the initiator.
 *
 * @param participants the participant ids
 * @param initiatorId the player to skip
 * @param response the line to deliver
 */
void zappy::server::ai::AiDispatcher::sendToAllExceptInitiator(
    const std::vector<int> &participants, int initiatorId,
    const std::string &response)
{
    for (int id : participants)
        if (id != initiatorId)
            _onResponse(id, response);
}

/**
 * @brief Run the pre-execute handler bound to @p kind, if any.
 *
 * @param playerId player starting the command
 * @param kind command about to be scheduled
 */
void zappy::server::ai::AiDispatcher::runPreHandler(
    int playerId, protocol::ai::CommandKind kind)
{
    const PreHandler pre = PRE_HANDLERS[static_cast<std::size_t>(kind)];
    if (pre)
        (this->*pre)(playerId);
}

/**
 * @brief Fire the fork-started notification at command scheduling time.
 *
 * @param playerId player starting the Fork
 */
void zappy::server::ai::AiDispatcher::preExecuteFork(int playerId)
{
    _world.notifyForkStarted(playerId);
}

/**
 * @brief Finish a scheduled command then advance the queue.
 *
 * @param playerId player whose command completed
 * @param cmd the command that just elapsed
 */
void zappy::server::ai::AiDispatcher::onCommandComplete(int playerId,
    protocol::ai::ParsedCommand cmd)
{
    const auto it = _states.find(playerId);
    if (it == _states.end())
        return;
    it->second.currentEventId = 0;
    respond(playerId, cmd);
    startNextCommand(playerId);
}

/**
 * @brief Execute @p cmd and deliver its response to @p playerId.
 *
 * @param playerId player to answer
 * @param cmd command to execute
 */
void zappy::server::ai::AiDispatcher::respond(int playerId,
    const protocol::ai::ParsedCommand &cmd)
{
    _onResponse(playerId, executeCommand(playerId, cmd));
}

/**
 * @brief Dispatch @p cmd through the per-kind handler table.
 *
 * @param playerId player issuing the command
 * @param cmd command to execute
 * @return std::string the response line ("ko" if the kind is out of range)
 */
std::string zappy::server::ai::AiDispatcher::executeCommand(int playerId,
    const protocol::ai::ParsedCommand &cmd)
{
    static constexpr std::array<Handler, protocol::ai::COMMAND_KIND_COUNT>
        handlers = {
            &AiDispatcher::executeForward, &AiDispatcher::executeRight,
            &AiDispatcher::executeLeft, &AiDispatcher::executeLook,
            &AiDispatcher::executeInventory, &AiDispatcher::executeBroadcast,
            &AiDispatcher::executeConnectNbr, &AiDispatcher::executeFork,
            &AiDispatcher::executeEject, &AiDispatcher::executeTake,
            &AiDispatcher::executeSet, &AiDispatcher::executeIncantation};
    const std::size_t index = static_cast<std::size_t>(cmd.kind);
    std::string result = "ko";
    if (index < handlers.size())
        result = (this->*handlers[index])(playerId, cmd.argument);
    return result;
}

/**
 * @brief Step one tile forward unless the target tile blocks movement.
 *
 * @param playerId player to move
 * @return std::string "ok" on success, "ko" if the target tile is impassable
 */
std::string zappy::server::ai::AiDispatcher::executeForward(
    int playerId, const std::string &)
{
    const game::Player &player = _world.player(playerId);
    const game::Position delta =
        game::OrientationHelper::forwardDelta(player.orientation());
    const game::Position newPos =
        (player.position() + delta).normalized(_world.width(),
            _world.height());
    const game::Tile &targetTile = _world.tileAt(newPos);
    std::string response = "ok";

    if (targetTile.isFlooded()
        || !game::biomeInfoFor(targetTile.biome()).traversable)
        response = "ko";
    else
        _world.movePlayer(playerId, newPos);
    return response;
}

/**
 * @brief Turn the player a clockwise quarter turn.
 *
 * @param playerId player to rotate
 * @return std::string always "ok"
 */
std::string zappy::server::ai::AiDispatcher::executeRight(
    int playerId, const std::string &)
{
    const game::Orientation newOrient = game::OrientationHelper::rotateRight(
        _world.player(playerId).orientation());
    _world.rotatePlayer(playerId, newOrient);
    return "ok";
}

/**
 * @brief Turn the player a counter-clockwise quarter turn.
 *
 * @param playerId player to rotate
 * @return std::string always "ok"
 */
std::string zappy::server::ai::AiDispatcher::executeLeft(
    int playerId, const std::string &)
{
    const game::Orientation newOrient = game::OrientationHelper::rotateLeft(
        _world.player(playerId).orientation());
    _world.rotatePlayer(playerId, newOrient);
    return "ok";
}

/**
 * @brief Render the player's vision cone as the Look response.
 *
 * @param playerId player whose vision is rendered
 * @return std::string the bracketed tile list
 */
std::string zappy::server::ai::AiDispatcher::executeLook(
    int playerId, const std::string &)
{
    return _lookBuilder.buildFor(playerId);
}

/**
 * @brief Format the player's inventory as "[food N, linemate N, ...]".
 *
 * @param playerId ID of the player whose inventory is queried
 * @return Inventory string in the AI protocol format
 */
std::string zappy::server::ai::AiDispatcher::executeInventory(
    int playerId, const std::string &)
{
    const server::game::Player &player = _world.player(playerId);
    std::string out = "[";
    bool first = true;

    for (std::size_t index = 0; index < game::RESOURCE_COUNT; ++index) {
        if (!first)
            out.append(", ");
        first = false;
        out.append(game::RESOURCE_NAMES[index]);
        out.append(" ");
        out.append(std::to_string(player.resource(
            static_cast<server::game::ResourceType>(index))));
    }
    out.append("]");
    return out;
}

/**
 * @brief Broadcast @p arg to every other living player, then notify the GUI.
 *
 * @param playerId the broadcasting player
 * @param arg the broadcast text
 * @return std::string always "ok"
 */
std::string zappy::server::ai::AiDispatcher::executeBroadcast(
    int playerId, const std::string &arg)
{
    broadcastToOthers(playerId, arg);
    return "ok";
}

/**
 * @brief Route a broadcast to every other living player with its K value.
 *
 * @param senderId the broadcasting player
 * @param text the broadcast text
 */
void zappy::server::ai::AiDispatcher::broadcastToOthers(
    int senderId, const std::string &text)
{
    const game::Player &sender = _world.player(senderId);
    const game::Position senderPos = sender.position();
    const int width = _world.width();
    const int height = _world.height();

    _world.forEachPlayer([&](int receiverId, const game::Player &receiver) {
        if (receiverId == senderId
            || receiver.state() == game::PlayerState::DEAD)
            return;
        const int k = game::BroadcastDirection::compute(senderPos,
            receiver.position(), receiver.orientation(), width, height);
        _onResponse(receiverId,
            "message " + std::to_string(k) + ", " + text);
    });
    _world.notifyBroadcast(senderId, text);
}

/**
 * @brief Report the number of waiting eggs for the player's team.
 *
 * @param playerId player issuing the command
 * @return std::string the waiting egg count as text
 */
std::string zappy::server::ai::AiDispatcher::executeConnectNbr(
    int playerId, const std::string &)
{
    const game::Player &player = _world.player(playerId);
    return std::to_string(_world.waitingEggCount(player.team()));
}

/**
 * @brief Lay a new egg at the player's tile and grow its team capacity.
 *
 * @param playerId player issuing the Fork
 * @return std::string always "ok"
 */
std::string zappy::server::ai::AiDispatcher::executeFork(
    int playerId, const std::string &)
{
    const game::Player &player = _world.player(playerId);
    _world.addEgg(player.team(), player.position(), playerId);
    _world.growTeam(player.team());
    return "ok";
}

/**
 * @brief Eject command handler (stub; real effect lands in ZAP-26).
 *
 * @return std::string always "ok"
 */
std::string zappy::server::ai::AiDispatcher::executeEject(
    int playerId, const std::string &)
{
    _world.notifyPlayerEjected(playerId);
    return "ok";
}

/**
 * @brief Pick up one unit of @p arg from the player's tile.
 *
 * @param playerId player taking the resource
 * @param arg resource name to take
 * @return std::string "ok" on success, "ko" on unknown name or empty tile
 */
std::string zappy::server::ai::AiDispatcher::executeTake(
    int playerId, const std::string &arg)
{
    std::string response = "ko";
    const auto type = game::ResourceNameResolver::resolve(arg);

    if (type && _world.takeResourceFromTile(playerId, *type))
        response = "ok";
    return response;
}

/**
 * @brief Drop one unit of @p arg from the player's inventory onto its tile.
 *
 * @param playerId player dropping the resource
 * @param arg resource name to drop
 * @return std::string "ok" on success, "ko" on unknown name or empty inventory
 */
std::string zappy::server::ai::AiDispatcher::executeSet(
    int playerId, const std::string &arg)
{
    std::string response = "ko";
    const auto type = game::ResourceNameResolver::resolve(arg);

    if (type && _world.dropResourceOnTile(playerId, *type))
        response = "ok";
    return response;
}

/**
 * @brief Incantation command handler (stub; real effect lands in ZAP-26).
 *
 * @return std::string always "ok"
 */
std::string zappy::server::ai::AiDispatcher::executeIncantation(
    int, const std::string &)
{
    return "ok";
}
