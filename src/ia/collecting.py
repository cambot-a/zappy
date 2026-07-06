##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## COLLECTING state handler
##

from fsm_helpers import request_look, tile_world_target
from navigation import navigate_to
from player_state import PlayerState, apply_forward, apply_right
from sender import CommandSender
from stones import needed_stones, STONE_NAMES

ROAM_TURN_INTERVAL = 4


def find_needed_stone_tile(
        state: PlayerState,
        wanted: dict[str, int]) -> tuple[int, str] | None:
    """Return (tile_index, stone_name) of the nearest needed stone, or None.

    Scans last_look in tile-index order. Only returns a stone that is
    present in the wanted dict (over-collection prevention).
    """
    for idx, tile in enumerate(state.last_look):
        for stone in STONE_NAMES:
            if stone in tile and stone in wanted:
                return idx, stone
    return None


def tick_collecting(state: PlayerState, sender: CommandSender,
                    roam_counter: list[int]) -> None:
    """Execute one FSM tick in the COLLECTING state.

    Priority actions:
    1. Needed stone on current tile: take it.
    2. Needed stone visible on another tile: navigate there.
    3. No Look result available: request one.
    4. Nothing useful visible: roam (forward + periodic right turn).

    The roam_counter list holds a single integer mutated in place
    across ticks.
    """
    if not sender.can_send:
        return

    wanted = needed_stones(state.level, state.inventory)
    if not wanted:
        return

    if state.last_look:
        for stone in STONE_NAMES:
            if stone in state.last_look[0] and stone in wanted:
                state.last_look[0].remove(stone)
                def _take_stone(r: str, s: str = stone) -> None:
                    if r == "ok":
                        state.inventory[s] = state.inventory.get(s, 0) + 1
                sender.send(f"Take {stone}", callback=_take_stone)
                return
        from fsm_helpers import SAFE_FOOD
        if "food" in state.last_look[0] and state.inventory.get("food", 0) < SAFE_FOOD:
            state.last_look[0].remove("food")
            def _take_food(r: str) -> None:
                if r == "ok":
                    state.inventory["food"] = state.inventory.get("food", 0) + 1
            sender.send("Take food", callback=_take_food)
            return

    if state.last_look:
        match = find_needed_stone_tile(state, wanted)
        if match is not None:
            idx, stone = match
            tx, ty = tile_world_target(state, idx)
            navigate_to(state, sender, tx, ty)
            state.last_look = []
            state.pending_look = False
            return

    if not state.last_look:
        request_look(state, sender)
        return

    roam_counter[0] += 1
    if roam_counter[0] % ROAM_TURN_INTERVAL == 0:
        sender.send("Right",
                    callback=lambda r: apply_right(state) if r == "ok" else None)
    else:
        sender.send("Forward",
                    callback=lambda r: apply_forward(state) if r == "ok" else None)
    state.last_look = []
    request_look(state, sender)
