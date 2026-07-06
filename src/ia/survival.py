##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## CRITICAL_SURVIVAL state handler
##

from fsm_helpers import find_food_tile, request_look, tile_world_target
from navigation import navigate_to
from player_state import PlayerState, apply_forward
from sender import CommandSender


def tick_critical_survival(state: PlayerState, sender: CommandSender) -> None:
    """Execute one FSM tick in the CRITICAL_SURVIVAL state.

    Priority actions:
    1. Food on current tile: take it immediately.
    2. Food visible elsewhere in Look result: navigate there.
    3. No Look result available: request one.
    4. Nothing visible: move forward one step and request a new Look.
    """
    if not sender.can_send:
        return

    if state.last_look and "food" in state.last_look[0]:
        state.last_look[0].remove("food")
        def _take_food(r: str) -> None:
            if r == "ok":
                state.inventory["food"] = state.inventory.get("food", 0) + 1
        sender.send("Take food", callback=_take_food)
        return

    if state.last_look:
        food_idx = find_food_tile(state)
        if food_idx is not None and food_idx > 0:
            tx, ty = tile_world_target(state, food_idx)
            navigate_to(state, sender, tx, ty)
            state.last_look = []
            state.pending_look = False
            return

    if not state.last_look:
        request_look(state, sender)
        return

    if sender.send("Forward"):
        apply_forward(state)
    request_look(state, sender)
