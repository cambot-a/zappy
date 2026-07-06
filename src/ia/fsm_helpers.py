##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Shared FSM helpers: inventory/look polling and food tile search
##

from inventory import parse_inventory
from look import parse_look, tile_index_to_offset
from player_state import Orientation, PlayerState
from sender import CommandSender

CRITICAL_FOOD = 3

SAFE_FOOD = 20

LEAD_FOOD = 20

LEAD_ABORT_FOOD = 5

FOLLOW_FOOD = 4

INVENTORY_POLL_INTERVAL = 20


def on_inventory_response(state: PlayerState, line: str) -> None:
    """Update inventory from a server Inventory response line."""
    state.pending_inventory = False
    parsed = parse_inventory(line)
    if parsed is not None:
        state.inventory = parsed


def on_look_response(state: PlayerState, line: str) -> None:
    """Update last_look from a server Look response line."""
    state.pending_look = False
    parsed = parse_look(line, state.level)
    if parsed is not None:
        state.last_look = parsed


def poll_inventory(state: PlayerState, sender: CommandSender,
                   tick: int) -> bool:
    """Issue an Inventory command if the poll interval has elapsed.

    Does nothing if an Inventory is already in flight or the pipeline
    is full. Returns True if sent.
    """
    if state.pending_inventory:
        return False
    if tick % INVENTORY_POLL_INTERVAL != 0:
        return False
    if not sender.can_send:
        return False
    state.pending_inventory = True
    sender.send("Inventory",
                callback=lambda line: on_inventory_response(state, line))
    return True


def find_food_tile(state: PlayerState) -> int | None:
    """Return the tile index of the nearest food tile in last_look, or None."""
    for idx, tile in enumerate(state.last_look):
        if "food" in tile:
            return idx
    return None


def request_look(state: PlayerState, sender: CommandSender) -> bool:
    """Issue a Look command if none is already in flight. Returns True if sent."""
    if state.pending_look or not sender.can_send:
        return False
    state.pending_look = True
    sender.send("Look",
                callback=lambda line: on_look_response(state, line))
    return True


def tile_world_target(state: PlayerState, idx: int) -> tuple[int, int]:
    """Convert a Look tile index to absolute world (tx, ty) coordinates.

    tile_index_to_offset returns (dx, dy) in the player's LOCAL frame
    where dy > 0 is forward and dx > 0 is right. This rotates those into
    the world frame based on the player's current orientation, then adds
    them to the player's position with toroidal wrap.

    NORTH: forward = -y, right = +x    world (dx_l, -dy_l)
    EAST:  forward = +x, right = +y    world (dy_l,  dx_l)
    SOUTH: forward = +y, right = -x    world (-dx_l, dy_l)
    WEST:  forward = -x, right = -y    world (-dy_l, -dx_l)
    """
    dx_l, dy_l = tile_index_to_offset(idx, state.level)
    o = state.orientation
    if o == Orientation.NORTH:
        dx, dy = dx_l, -dy_l
    elif o == Orientation.EAST:
        dx, dy = dy_l, dx_l
    elif o == Orientation.SOUTH:
        dx, dy = -dx_l, dy_l
    else:
        dx, dy = -dy_l, -dx_l
    return (state.x + dx) % state.map_width, (state.y + dy) % state.map_height
