/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Per-player AI command queue, scheduling and dispatch
*/

#ifndef SERVER_AI_AIDISPATCHER_HPP_
    #define SERVER_AI_AIDISPATCHER_HPP_

    #include <deque>
    #include <functional>
    #include <string>
    #include <string_view>
    #include <unordered_map>
    #include <vector>

    #include "server/ai/LookResponseBuilder.hpp"
    #include "server/game/ElevationRules.hpp"
    #include "server/game/World.hpp"
    #include "server/scheduler/Scheduler.hpp"
    #include "protocol/ai/CommandParser.hpp"

namespace zappy::server::ai {

/**
 * @brief Parses, queues, time-schedules and executes AI commands per player.
 */
class AiDispatcher {
public:
    using OnResponse = std::function<void(int, std::string)>;

    /**
     * @brief Wire the dispatcher to the world, scheduler and response sink.
     *
     * @param world game world the commands act upon
     * @param scheduler event queue timing the commands
     * @param frequency reciprocal of the time unit (the f config value)
     * @param onResponse callback delivering a response line to a player
     */
    AiDispatcher(game::World &world, scheduler::Scheduler &scheduler,
        int frequency, OnResponse onResponse);

    AiDispatcher(const AiDispatcher &) = delete;
    AiDispatcher &operator=(const AiDispatcher &) = delete;
    AiDispatcher(AiDispatcher &&) = delete;
    AiDispatcher &operator=(AiDispatcher &&) = delete;

    /**
     * @brief Parse and enqueue @p line for @p playerId.
     *
     * Malformed or unknown lines yield an immediate "ko". A full queue drops
     * the command silently.
     *
     * @param playerId player issuing the command
     * @param line raw command line
     */
    void dispatch(int playerId, std::string_view line);

    /**
     * @brief Cancel the in-flight command and drop the queue for @p playerId.
     *
     * @param playerId player to stop; no-op if unknown
     */
    void stopPlayer(int playerId) noexcept;

    /**
     * @brief Force-start an incantation on @p playerId at the GUI admin's
     * request.
     *
     * Validates the standard prerequisites (level, headcount, stones) then
     * launches the normal ritual, which finishes on its own after the regular
     * incantation delay.
     *
     * @param playerId the player to elevate
     * @return bool true if the ritual was started, false if prerequisites fail
     */
    bool adminStartIncantation(int playerId);

    /**
     * @brief Force-abort the incantation @p playerId is currently performing.
     *
     * Cancels the scheduled completion and ends the ritual as a failure for
     * every frozen participant, restoring their command queues.
     *
     * @param playerId the incanting initiator
     * @return bool true if an incantation was stopped, false otherwise
     */
    bool adminStopIncantation(int playerId);

    /**
     * @brief Update the time frequency.
     *
     * @param frequency new time frequency
     */
    void setFrequency(int frequency) noexcept;

private:
    using Handler = std::string (AiDispatcher::*)(int, const std::string &);
    using PreHandler = void (AiDispatcher::*)(int);

    /**
     * @brief Per-player command queue plus its in-flight event id.
     */
    struct PlayerState {
        std::deque<protocol::ai::ParsedCommand> queue;
        scheduler::EventId currentEventId = 0;
        bool forcedIncant = false;
    };

    static constexpr std::size_t MAX_PENDING_COMMANDS = 10;

    /**
     * @brief Start as many queued commands as resolve instantly, schedule one.
     *
     * @param playerId player whose queue is advanced
     */
    void startNextCommand(int playerId);

    /**
     * @brief Whether the next queued command may start now.
     *
     * @param playerId player whose queue is inspected
     * @return bool true if a command is pending and the player is not frozen
     */
    [[nodiscard]] bool canStartNextCommand(int playerId) const;

    /**
     * @brief Pop the front command and route it to the right starter.
     *
     * @param playerId player whose queue advances
     */
    void processNextQueuedCommand(int playerId);

    /**
     * @brief Schedule (or instantly run) a non-incantation command.
     *
     * @param playerId player issuing the command
     * @param cmd command to start
     */
    void startStandardCommand(int playerId,
        const protocol::ai::ParsedCommand &cmd);

    /**
     * @brief Validate prerequisites then launch or reject an incantation.
     *
     * @param playerId the initiating player
     */
    void startIncantation(int playerId);

    /**
     * @brief Ids of every ALIVE player standing on the initiator's tile.
     *
     * @param playerId the initiating player
     * @return std::vector<int> the participant ids (includes the initiator)
     */
    [[nodiscard]] std::vector<int> collectParticipants(int playerId) const;

    /**
     * @brief Ids of every player frozen (INCANTING) on @p playerId's tile.
     *
     * Used to recover the participants of an in-flight ritual when the GUI
     * admin forces it to stop.
     *
     * @param playerId the incanting initiator
     * @return std::vector<int> the frozen participant ids
     */
    [[nodiscard]] std::vector<int> frozenParticipantsOnTile(
        int playerId) const;

    /**
     * @brief Whether @p playerId may elevate with @p participants right now.
     *
     * @param playerId the initiating player
     * @param participants alive players on the tile
     * @return bool true if level, headcount and stones all qualify
     */
    [[nodiscard]] bool checkPrerequisites(int playerId,
        const std::vector<int> &participants) const;

    /**
     * @brief Whether the tile holds every stone @p rule demands.
     *
     * @param pos the tile position
     * @param rule the elevation rule to satisfy
     * @return bool true if all stones are present in sufficient quantity
     */
    [[nodiscard]] bool tileHasStones(game::Position pos,
        const game::ElevationRule &rule) const;

    /**
     * @brief Whether the player exists and is not DEAD.
     *
     * @param playerId player to test
     * @return bool true if alive or incanting
     */
    [[nodiscard]] bool isPlayerAlive(int playerId) const;

    /**
     * @brief Freeze participants, announce the ritual and schedule its end.
     *
     * @param playerId the initiating player
     * @param participants alive players on the tile
     */
    void beginIncantationRitual(int playerId,
        const std::vector<int> &participants);

    /**
     * @brief Re-check prerequisites and apply success or failure.
     *
     * @param initiatorId the initiating player
     * @param participants the participants captured at ritual start
     */
    void finalizeIncantation(int initiatorId, std::vector<int> participants);

    /**
     * @brief Elevate every alive participant and consume the tile's stones.
     *
     * @param initiatorId the initiating player
     * @param alive participants still alive at finalization
     * @param newLevel the resulting level
     */
    void applySuccess(int initiatorId, const std::vector<int> &alive,
        int newLevel);

    /**
     * @brief Reject the incantation: answer "ko" and unfreeze participants.
     *
     * @param initiatorId the initiating player
     * @param alive participants still alive at finalization
     */
    void applyFailure(int initiatorId, const std::vector<int> &alive);

    /**
     * @brief Remove the stones @p rule demands from @p tilePos.
     *
     * @param tilePos the tile to deduct from
     * @param rule the elevation rule that was satisfied
     */
    void consumeStones(game::Position tilePos, const game::ElevationRule &rule);

    /**
     * @brief Unfreeze every listed participant.
     *
     * @param participants players to unfreeze
     */
    void unfreezeAll(const std::vector<int> &participants);

    /**
     * @brief Resume each participant's queue after the ritual.
     *
     * @param participants players whose queues advance
     */
    void resumeParticipantQueues(const std::vector<int> &participants);

    /**
     * @brief Send @p response to every participant except the initiator.
     *
     * @param participants the participant ids
     * @param initiatorId the player to skip
     * @param response the line to deliver
     */
    void sendToAllExceptInitiator(const std::vector<int> &participants,
        int initiatorId, const std::string &response);

    /**
     * @brief Run the pre-execute handler bound to @p kind, if any.
     *
     * @param playerId player starting the command
     * @param kind command about to be scheduled
     */
    void runPreHandler(int playerId, protocol::ai::CommandKind kind);

    /**
     * @brief Fire the fork-started notification at command scheduling time.
     *
     * @param playerId player starting the Fork
     */
    void preExecuteFork(int playerId);


    static constexpr std::array<PreHandler, protocol::ai::COMMAND_KIND_COUNT>
        PRE_HANDLERS = {
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            &AiDispatcher::preExecuteFork,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        };

    /**
     * @brief Finish a scheduled command then advance the queue.
     *
     * @param playerId player whose command completed
     * @param cmd the command that just elapsed
     */
    void onCommandComplete(int playerId, protocol::ai::ParsedCommand cmd);

    /**
     * @brief Execute @p cmd and deliver its response to @p playerId.
     *
     * @param playerId player to answer
     * @param cmd command to execute
     */
    void respond(int playerId, const protocol::ai::ParsedCommand &cmd);

    /**
     * @brief Run the handler bound to @p cmd and return its response line.
     *
     * @param playerId player issuing the command
     * @param cmd command to execute
     * @return std::string the response line (no trailing newline)
     */
    [[nodiscard]] std::string executeCommand(int playerId,
        const protocol::ai::ParsedCommand &cmd);

    /**
     * @brief Route a broadcast to every other living player with its K value.
     *
     * @param senderId the broadcasting player
     * @param text the broadcast text
     */
    void broadcastToOthers(int senderId, const std::string &text);

    std::string executeForward(int playerId, const std::string &arg);
    std::string executeRight(int playerId, const std::string &arg);
    std::string executeLeft(int playerId, const std::string &arg);
    std::string executeLook(int playerId, const std::string &arg);
    std::string executeInventory(int playerId, const std::string &arg);
    std::string executeBroadcast(int playerId, const std::string &arg);
    std::string executeConnectNbr(int playerId, const std::string &arg);
    std::string executeFork(int playerId, const std::string &arg);
    std::string executeEject(int playerId, const std::string &arg);
    std::string executeTake(int playerId, const std::string &arg);
    std::string executeSet(int playerId, const std::string &arg);
    std::string executeIncantation(int playerId, const std::string &arg);

    game::World &_world;
    scheduler::Scheduler &_scheduler;
    int _frequency;
    OnResponse _onResponse;
    std::unordered_map<int, PlayerState> _states;
    LookResponseBuilder _lookBuilder;
};

} // namespace zappy::server::ai

#endif /* !SERVER_AI_AIDISPATCHER_HPP_ */
