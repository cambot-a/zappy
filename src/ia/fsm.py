##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## FSM orchestrator: priority evaluator and main tick loop
##

from collecting import tick_collecting
from coordination import RallyInfo, handle_coordination_broadcast, tick_coordinating
from elevation import ElevationState, ElevationStatus
from foraging import tick_foraging
from forking import can_fork, tick_forking
from fsm_helpers import CRITICAL_FOOD, SAFE_FOOD, poll_inventory
from have_broadcast import TeamMemberState, broadcast_have, on_broadcast_received
from leading_elevation import (LeaderInfo, can_lead_elevation,
                               handle_leader_broadcast, tick_leading_elevation)
from player_state import FsmState, PlayerState
from scouting import tick_scouting
from sender import CommandSender
from stones import needed_stones
from survival import tick_critical_survival


def evaluate_fsm_state(state: PlayerState, fork_cooldown: int = 0,
                       lead_cooldown: int = 0) -> FsmState:
    """Return the highest-priority FSM state that applies this tick.

    Evaluates conditions in strict priority order. The ELEVATING state
    is set externally by the elevation dispatcher, not here.
    """
    food = state.inventory.get("food", 0)
    if food < CRITICAL_FOOD:
        return FsmState.CRITICAL_SURVIVAL
    if state.fsm_state == FsmState.ELEVATING:
        return FsmState.ELEVATING
    if food < SAFE_FOOD:
        return FsmState.FORAGING
    if lead_cooldown == 0 and can_lead_elevation(state):
        return FsmState.LEADING_ELEVATION
    if needed_stones(state.level, state.inventory):
        return FsmState.COLLECTING
    if can_fork(state, fork_cooldown):
        return FsmState.FORKING
    return FsmState.SCOUTING


class Fsm:
    """Priority-driven finite state machine for one AI player.

    Attributes:
        team_members: Last-known teammate states keyed by broadcast direction.
    """

    def __init__(self, state: PlayerState) -> None:
        """Bind the FSM to the player state."""
        self._state = state
        self._tick: int = 0
        self._roam_counter: list[int] = [0]
        self._rally: list[RallyInfo | None] = [None]
        self._leader: list[LeaderInfo | None] = [None]
        self._scout_counter: list[int] = [0]
        self._fork_cooldown: list[int] = [0]
        self._lead_cooldown: list[int] = [0]
        self.team_members: dict[int, TeamMemberState] = {}

    def on_broadcast(self, sender: CommandSender,
                     direction: int, text: str,
                     elev: "ElevationState | None" = None) -> None:
        """Handle an incoming broadcast: update team table and check RALLY/READY."""
        on_broadcast_received(self.team_members, direction, text)
        handle_coordination_broadcast(
            self._state, self._rally, sender, direction, text,
            leader_holder=self._leader)
        from elevation import ElevationState as _ES
        _elev = elev if elev is not None else _ES()
        handle_leader_broadcast(self._state, self._leader, sender, _elev, text)

    def tick(self, sender: CommandSender, elev: ElevationState) -> None:
        """Run one FSM iteration.

        Called by the event loop after every server line is dispatched.
        """
        self._tick += 1
        poll_inventory(self._state, sender, self._tick)
        if self._state.inventory.get("food", 0) >= SAFE_FOOD:
            broadcast_have(self._state, sender, self._tick)

        if elev.status == ElevationStatus.SUCCESS:
            self._state.level = elev.new_level
            if self._leader[0] is not None and sender.can_send:
                from broadcast_protocol import MSG_CANCEL, format_msg as _fmt
                sender.send(f"Broadcast {_fmt(MSG_CANCEL)}",
                            callback=lambda r: None)
            self._state.fsm_state = FsmState.COLLECTING
            self._leader[0] = None
            self._rally[0] = None
            elev.status = ElevationStatus.IDLE
            self._lead_cooldown[0] = 60

        if elev.status == ElevationStatus.FAILED:
            if sender.can_send:
                from broadcast_protocol import MSG_CANCEL, format_msg as _fmt
                sender.send(f"Broadcast {_fmt(MSG_CANCEL)}",
                            callback=lambda r: None)
            self._leader[0] = None
            self._rally[0] = None
            self._state.fsm_state = FsmState.COLLECTING
            elev.status = ElevationStatus.IDLE

        if elev.status == ElevationStatus.IN_PROGRESS:
            self._state.fsm_state = FsmState.ELEVATING
            return

        if self._fork_cooldown[0] > 0:
            self._fork_cooldown[0] -= 1

        if self._lead_cooldown[0] > 0:
            self._lead_cooldown[0] -= 1

        new_state = evaluate_fsm_state(self._state, self._fork_cooldown[0],
                                       self._lead_cooldown[0])

        if self._rally[0] is not None:
            from fsm_helpers import FOLLOW_FOOD as _ff
            low_food = new_state == FsmState.FORAGING and \
                self._state.inventory.get("food", 0) < _ff
            if new_state not in (FsmState.CRITICAL_SURVIVAL,
                                 FsmState.ELEVATING) and not low_food:
                new_state = FsmState.COORDINATING
            else:
                from coordination import COORDINATING_STALE_TICKS
                self._rally[0].ticks_stale += 1
                if self._rally[0].ticks_stale > COORDINATING_STALE_TICKS:
                    self._rally[0] = None

        if self._leader[0] is not None and new_state not in (
                FsmState.CRITICAL_SURVIVAL, FsmState.ELEVATING,
                FsmState.COORDINATING):
            from fsm_helpers import LEAD_ABORT_FOOD as _laf
            food_ok = self._state.inventory.get("food", 0) >= _laf
            stones_ok = not needed_stones(self._state.level, self._state.inventory)
            if food_ok and stones_ok:
                new_state = FsmState.LEADING_ELEVATION
            else:
                from broadcast_protocol import MSG_CANCEL, format_msg as _fmt
                if sender.can_send:
                    sender.send(f"Broadcast {_fmt(MSG_CANCEL)}",
                                callback=lambda r: None)
                self._leader[0] = None

        if new_state == FsmState.CRITICAL_SURVIVAL and self._leader[0] is not None:
            if sender.can_send:
                from broadcast_protocol import MSG_CANCEL, format_msg as _fmt
                sender.send(f"Broadcast {_fmt(MSG_CANCEL)}",
                            callback=lambda r: None)
            self._leader[0] = None
        self._state.fsm_state = new_state

        if new_state == FsmState.CRITICAL_SURVIVAL:
            tick_critical_survival(self._state, sender)
        elif new_state == FsmState.FORAGING:
            tick_foraging(self._state, sender)
        elif new_state == FsmState.COLLECTING:
            tick_collecting(self._state, sender, self._roam_counter)
        elif new_state == FsmState.COORDINATING:
            tick_coordinating(self._state, sender, self._rally)
        elif new_state == FsmState.LEADING_ELEVATION:
            had_leader = self._leader[0] is not None
            if self._leader[0] is None:
                self._leader[0] = LeaderInfo()
            tick_leading_elevation(self._state, sender, self._leader, elev)
            if had_leader and self._leader[0] is None:
                self._lead_cooldown[0] = 150
        elif new_state == FsmState.SCOUTING:
            tick_scouting(self._state, sender, self._scout_counter)
        elif new_state == FsmState.FORKING:
            tick_forking(self._state, sender, self._fork_cooldown)
