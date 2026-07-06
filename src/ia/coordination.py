##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## COORDINATING state handler (follower role in a joint incantation)
##

from dataclasses import dataclass

from broadcast_protocol import (MSG_CANCEL, MSG_COMING, MSG_RALLY, MSG_READY,
                                format_msg, parse_msg)
from player_state import FsmState, PlayerState, apply_forward, apply_left, apply_right
from sender import CommandSender


@dataclass(slots=True)
class RallyInfo:
    """State for the current follower rally.

    Navigation uses the broadcast direction K from the leader's repeated
    RALLY broadcasts rather than absolute coordinates (which are meaningless
    across different AIs' dead-reckoned coordinate frames).

    Attributes:
        level: Level required for the incantation.
        direction: K value from the most recent RALLY broadcast.
            0 means the leader is on the same tile.
        ready_sent: True after READY has been broadcast.  While True the
            follower stays frozen on the tile instead of navigating away.
        step_taken: True after a Forward has been queued for the current
            direction.  Reset when the direction changes OR when the
            previous Forward has executed (forward_pending=False).
            Limits the pipeline to at most 1 Forward per direction cycle.
        forward_pending: True from the moment a Forward is enqueued until
            its response arrives.  Used to gate READY: we only send READY
            when forward_pending=False (no in-flight Forward that could
            relocate us after READY but before the Incantation).
            Unlike in_flight, this flag is unaffected by Look/Inventory.
        ticks_stale: Ticks elapsed since the last RALLY refresh.  If this
            exceeds COORDINATING_STALE_TICKS the rally is considered
            abandoned by the leader and is cleared automatically.
    """

    level: int
    direction: int
    ready_sent: bool = False
    step_taken: bool = False
    forward_pending: bool = False
    ticks_stale: int = 0


COORDINATING_STALE_TICKS = 200


def _navigate_toward_k(state: PlayerState, sender: CommandSender,
                       k: int, rally: "RallyInfo | None" = None) -> None:
    """Take at most 2 turns + 1 Forward step toward broadcast source K."""
    from broadcast_direction import direction_to_orientation
    from navigation import turns_to_face

    target_orient = direction_to_orientation(k, state.orientation)
    if target_orient is None:
        return

    for cmd in turns_to_face(state.orientation, target_orient):
        if not sender.can_send:
            return
        if cmd == "Right":
            sender.send("Right", callback=lambda r: None)
            apply_right(state)
        else:
            sender.send("Left", callback=lambda r: None)
            apply_left(state)

    if sender.can_send:
        if rally is not None:
            rally.forward_pending = True
            def _on_fwd(r: str, rl: RallyInfo = rally) -> None:
                rl.forward_pending = False
            sender.send("Forward", callback=_on_fwd)
        else:
            sender.send("Forward", callback=lambda r: None)
        apply_forward(state)


def on_rally_received(
        state: PlayerState,
        rally_holder: list,
        leader_holder: list,
        direction: int,
        fields: tuple[str, ...],
        sender: CommandSender) -> None:
    """Process an incoming RALLY message (follower side).

    Accepts the rally only when the announced level matches the player's
    current level. On the first RALLY, transitions to COORDINATING and
    broadcasts COMING. On subsequent (repeated) RALLYs from the same
    level, just refreshes the direction so the follower can self-correct.

    If we are currently acting as a leader ourselves, we abandon that role
    and follow the incoming leader instead (prevents two-leader deadlock).

    RALLY field format: level:x:y  (x,y are the leader's position in its
    own coordinate frame and are ignored; navigation uses direction K).
    """
    if len(fields) != 3:
        return
    try:
        level = int(fields[0])
    except (ValueError, IndexError):
        return
    if level < state.level:
        return

    if leader_holder[0] is not None:
        if level == state.level and leader_holder[0].has_rallied:
            # Committed to a same-level rally; don't yield to prevent deadlock.
            return
        # Higher-level leader takes priority: yield even if already committed.
        if leader_holder[0].has_rallied and sender.can_send:
            sender.send(f"Broadcast {format_msg(MSG_CANCEL)}",
                        callback=lambda r: None)
        leader_holder[0] = None

    existing = rally_holder[0]
    if existing is not None and existing.level == level:
        if existing.ready_sent:
            return
        old_direction = existing.direction
        existing.direction = direction
        existing.ticks_stale = 0
        if direction == 0:
            if not existing.forward_pending and sender.can_send:
                sender.send(f"Broadcast {format_msg(MSG_READY)}",
                            callback=lambda r: None)
                existing.ready_sent = True
            return
        if direction != old_direction:
            existing.step_taken = False
        elif not existing.forward_pending:
            existing.step_taken = False
        return

    # Don't switch if already committed to any rally (READY sent)
    if existing is not None and existing.ready_sent:
        return
    # Don't downgrade from a higher-level rally to a lower one
    if existing is not None and level < existing.level:
        return

    rally_holder[0] = RallyInfo(level=level, direction=direction)
    state.fsm_state = FsmState.COORDINATING
    if sender.can_send:
        sender.send(f"Broadcast {format_msg(MSG_COMING)}",
                    callback=lambda r: None)


def handle_coordination_broadcast(
        state: PlayerState,
        rally_holder: list,
        sender: CommandSender,
        direction: int,
        text: str,
        leader_holder: "list | None" = None) -> None:
    """Route a received broadcast to the appropriate coordination handler.

    Only RALLY is handled here. CANCEL clears any active rally.
    Other types (COMING, READY, HAVE) are ignored by this handler.
    """
    msg = parse_msg(text)
    if msg is None:
        return
    _leader = leader_holder if leader_holder is not None else [None]
    if msg.msg_type == MSG_RALLY:
        on_rally_received(state, rally_holder, _leader, direction, msg.fields, sender)
    elif msg.msg_type == MSG_CANCEL:
        rally_holder[0] = None


def tick_coordinating(
        state: PlayerState,
        sender: CommandSender,
        rally_holder: list) -> None:
    """Execute one FSM tick in the COORDINATING state (follower role).

    Navigation uses the broadcast direction K from the leader's periodic
    RALLY broadcasts rather than absolute coordinates.

    Priority actions:
    1. READY already sent: freeze in place (Look keepalive only).
    2. direction == 0 (on same tile as leader): broadcast READY.
    3. Otherwise: take one step toward the broadcast source (K direction).
    """
    if not sender.can_send:
        return
    rally = rally_holder[0]
    if rally is None:
        return

    if rally.ready_sent:
        rally.ticks_stale += 1
        if rally.ticks_stale > COORDINATING_STALE_TICKS:
            rally_holder[0] = None
            return
        from fsm_helpers import request_look, SAFE_FOOD
        if state.last_look and "food" in state.last_look[0] \
                and state.inventory.get("food", 0) < SAFE_FOOD and sender.can_send:
            state.last_look[0].remove("food")
            def _eat(r: str) -> None:
                if r == "ok":
                    state.inventory["food"] = state.inventory.get("food", 0) + 1
            sender.send("Take food", callback=_eat)
            return
        if not state.pending_look:
            request_look(state, sender)
        return

    rally.ticks_stale += 1
    if rally.ticks_stale > COORDINATING_STALE_TICKS:
        rally_holder[0] = None
        return

    if rally.direction == 0:
        from fsm_helpers import request_look, SAFE_FOOD
        if state.last_look and "food" in state.last_look[0] \
                and state.inventory.get("food", 0) < SAFE_FOOD and sender.can_send:
            state.last_look[0].remove("food")
            def _eat2(r: str) -> None:
                if r == "ok":
                    state.inventory["food"] = state.inventory.get("food", 0) + 1
            sender.send("Take food", callback=_eat2)
            return
        if not state.pending_look:
            request_look(state, sender)
        return

    if not rally.step_taken and not rally.forward_pending:
        from fsm_helpers import SAFE_FOOD
        if state.last_look and "food" in state.last_look[0] \
                and state.inventory.get("food", 0) < SAFE_FOOD and sender.can_send:
            state.last_look[0].remove("food")
            def _eat_nav(r: str) -> None:
                if r == "ok":
                    state.inventory["food"] = state.inventory.get("food", 0) + 1
            sender.send("Take food", callback=_eat_nav)
            return
        _navigate_toward_k(state, sender, rally.direction, rally)
        rally.step_taken = True
    else:
        from fsm_helpers import request_look, SAFE_FOOD
        if state.last_look and "food" in state.last_look[0] \
                and state.inventory.get("food", 0) < SAFE_FOOD and sender.can_send:
            state.last_look[0].remove("food")
            def _eat_wait(r: str) -> None:
                if r == "ok":
                    state.inventory["food"] = state.inventory.get("food", 0) + 1
            sender.send("Take food", callback=_eat_wait)
            return
        if not state.pending_look:
            request_look(state, sender)
