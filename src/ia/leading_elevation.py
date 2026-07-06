##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## LEADING_ELEVATION state handler (leader role in a joint incantation)
##

from dataclasses import dataclass, field

from broadcast_protocol import MSG_CANCEL, MSG_RALLY, MSG_READY, format_msg, parse_msg
from elevation import ElevationState, ElevationStatus
from player_state import FsmState, PlayerState
from sender import CommandSender
from stones import ELEVATION_REQUIREMENTS, STONE_NAMES, needed_stones

RALLY_TIMEOUT_TICKS = 600

RALLY_REPEAT_INTERVAL = 3


@dataclass(slots=True)
class LeaderInfo:
    """State tracked by the leader during a rally-and-incantation sequence.

    Attributes:
        ready_count: Number of READY replies received so far.
        ticks_waiting: Ticks elapsed since the RALLY was broadcast.
        has_rallied: True after the first RALLY broadcast has been sent.
            A committed leader (has_rallied=True) will not yield to another
            AI's RALLY so that two simultaneous leaders don't both abandon.
    """

    ready_count: int = 0
    ticks_waiting: int = 0
    has_rallied: bool = False


def can_lead_elevation(state: PlayerState) -> bool:
    """Return True when this player can initiate an incantation as leader.

    Conditions (all must hold):
    - No stones are still needed (inventory is satisfied for this level).
    - Food is above LEAD_FOOD so the player won't die during the rally.
    - Level 1 requires only 1 player, so no coordination is needed.
    """
    from fsm_helpers import LEAD_FOOD
    if needed_stones(state.level, state.inventory):
        return False
    if state.inventory.get("food", 0) < LEAD_FOOD:
        return False
    req = ELEVATION_REQUIREMENTS.get(state.level, {})
    needed_players = req.get("players", 0)
    return needed_players >= 1


def deposit_stones(state: PlayerState, sender: CommandSender) -> int:
    """Enqueue Set commands to deposit all required stones on the tile.

    Only deposits stones listed in the elevation requirements for this level
    (does not drop surplus stones). Decrements the inventory optimistically
    on 'ok' so that needed_stones() reflects the true state after deposit.
    Returns the number of commands sent.
    """
    req = ELEVATION_REQUIREMENTS.get(state.level, {})
    sent = 0
    for stone in STONE_NAMES:
        qty = req.get(stone, 0)
        for _ in range(qty):
            if not sender.can_send:
                break
            def _on_set(r: str, s: str = stone) -> None:
                if r == "ok":
                    state.inventory[s] = max(0, state.inventory.get(s, 0) - 1)
            sender.send(f"Set {stone}", callback=_on_set)
            sent += 1
    return sent


def required_ready_count(level: int) -> int:
    """Return the number of READY replies needed before Incantation.

    Leader counts as 1, so followers needed = players_required - 1.
    """
    if level == 1:
        return 0
    req = ELEVATION_REQUIREMENTS.get(level, {})
    return max(0, req.get("players", 1) - 1)


def on_ready_received(
        state: PlayerState,
        leader_info: list,
        sender: CommandSender,
        elev: ElevationState) -> None:
    """Process a READY broadcast from a follower.

    Increments the ready counter. If enough followers are ready and the
    inventory is still satisfied, deposits stones and starts Incantation.

    Does NOT clear leader_info[0] so the leader keeps its freeze logic
    active until the Incantation result arrives (FSM clears leader_info on
    SUCCESS or FAILED).
    """
    info = leader_info[0]
    if info is None:
        return
    if elev.incantation_sent:
        return
    info.ready_count += 1
    needed = required_ready_count(state.level)
    if info.ready_count >= needed:
        deposit_stones(state, sender)
        if sender.can_send:
            elev.incantation_sent = True
            def _on_incant(r: str) -> None:
                if r == "ko":
                    elev.status = ElevationStatus.FAILED
                    elev.incantation_sent = False
            sender.send("Incantation", callback=_on_incant)


def handle_leader_broadcast(
        state: PlayerState,
        leader_info: list,
        sender: CommandSender,
        elev: ElevationState,
        text: str) -> None:
    """Route a received broadcast to the leader-side handler.

    Only READY is relevant here. All other types are ignored.
    """
    msg = parse_msg(text)
    if msg is None:
        return
    if msg.msg_type == MSG_READY:
        on_ready_received(state, leader_info, sender, elev)


def _startup_delay(state: PlayerState) -> int:
    """Return a position-based startup delay (ticks) before broadcasting RALLY.

    Each AI waits this many ticks after entering LEADING_ELEVATION before
    it commits to a RALLY broadcast. During the wait it listens for another
    AI's RALLY and yields if one is received.  Using a position-based value
    ensures different AIs have different delays, reducing simultaneous-broadcast
    collisions.
    """
    return 5 + (state.x * 3 + state.y) % 6


def tick_leading_elevation(
        state: PlayerState,
        sender: CommandSender,
        leader_info: list,
        elev: ElevationState) -> None:
    """Execute one FSM tick in the LEADING_ELEVATION state.

    Level 1 (solo): deposit stones and incantate immediately.
    Level 2+ (multi-player):
        Phase 1 startup listen (0 … startup_delay-1 ticks): send Look
            keepalives.  If a RALLY is received from another AI during this
            phase, the coordination handler will clear leader_info (yield).
        Phase 2 broadcast (tick == startup_delay): send RALLY broadcast.
        Phase 3 wait (startup_delay+1 … +RALLY_TIMEOUT_TICKS): keepalive.
        Timeout: broadcast CANCEL and abort.
    """
    info = leader_info[0]
    if info is None:
        return

    if not sender.can_send:
        return

    startup = _startup_delay(state)

    if elev.incantation_sent:
        from fsm_helpers import request_look
        if not state.pending_look:
            request_look(state, sender)
        return

    if required_ready_count(state.level) == 0:
        if info.ticks_waiting == 0:
            deposit_stones(state, sender)
            if sender.can_send:
                elev.incantation_sent = True
                def _on_incant(r: str) -> None:
                    if r == "ko":
                        elev.status = ElevationStatus.FAILED
                        elev.incantation_sent = False
                sender.send("Incantation", callback=_on_incant)
            leader_info[0] = None
        return

    ticks_after_start = info.ticks_waiting - startup
    if info.ticks_waiting >= startup and ticks_after_start % RALLY_REPEAT_INTERVAL == 0:
        text = format_msg(MSG_RALLY, str(state.level),
                          str(state.x), str(state.y))
        sender.send(f"Broadcast {text}", callback=lambda r: None)
        info.has_rallied = True

    info.ticks_waiting += 1

    if info.ticks_waiting > startup + RALLY_TIMEOUT_TICKS:
        sender.send(f"Broadcast {format_msg(MSG_CANCEL)}",
                    callback=lambda r: None)
        leader_info[0] = None
        return

    from fsm_helpers import request_look
    if state.last_look and "food" in state.last_look[0] and sender.can_send:
        state.last_look[0].remove("food")
        def _eat(r: str) -> None:
            if r == "ok":
                state.inventory["food"] = state.inventory.get("food", 0) + 1
        sender.send("Take food", callback=_eat)
        return
    if not state.pending_look:
        request_look(state, sender)
