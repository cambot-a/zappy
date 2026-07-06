##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## FORAGING state handler
##

from fsm_helpers import find_food_tile, request_look, tile_world_target
from navigation import navigate_to
from player_state import PlayerState, apply_forward
from sender import CommandSender


def tick_foraging(state: PlayerState, sender: CommandSender) -> None:
    """Execute one FSM tick in the FORAGING state.

    Opportunistically collects food when below the safe threshold.
    Yields immediately to CRITICAL_SURVIVAL if food drops further.

    Priority actions:
    1. Food on current tile: take it.
    2. Food visible in Look result: navigate there.
    3. Nothing visible: advance one step and request a new Look.
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
        from stones import needed_stones, STONE_NAMES
        wanted = needed_stones(state.level, state.inventory)
        for stone in STONE_NAMES:
            if stone in state.last_look[0] and stone in wanted:
                state.last_look[0].remove(stone)
                def _take_stone(r: str, s: str = stone) -> None:
                    if r == "ok":
                        state.inventory[s] = state.inventory.get(s, 0) + 1
                sender.send(f"Take {stone}", callback=_take_stone)
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
