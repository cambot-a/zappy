##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Player state
##

from dataclasses import dataclass, field
from enum import Enum, auto

from inventory import Inventory, empty_inventory
from torus import torus_wrap


class Orientation(Enum):
    """Cardinal direction the player is currently facing.

    Values are chosen to allow simple clockwise/counter-clockwise rotation
    arithmetic using modular indexing.
    """

    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3


class FsmState(Enum):
    """FSM state of the player, evaluated in decreasing priority order.

    Attributes:
        CRITICAL_SURVIVAL: Food below hard threshold; all other concerns dropped.
        ELEVATING: Frozen during an incantation ritual.
        LEADING_ELEVATION: All stones collected; coordinating and initiating.
        COORDINATING: Navigating to a rally point or waiting for followers.
        FORAGING: Food below safe threshold; opportunistic collection.
        COLLECTING: Navigating toward needed stones.
        FORKING: Laying an egg to expand team slots.
        SCOUTING: Default exploration state.
    """

    CRITICAL_SURVIVAL = auto()
    ELEVATING = auto()
    LEADING_ELEVATION = auto()
    COORDINATING = auto()
    FORAGING = auto()
    COLLECTING = auto()
    FORKING = auto()
    SCOUTING = auto()


@dataclass
class PlayerState:
    """Complete mutable state of the AI player.

    This object is created once after the handshake and passed through
    the entire FSM. All state transitions and sensor updates modify it
    in place.

    Attributes:
        level: Current elevation level (1-8).
        inventory: Resource counts including food.
        x: Self-tracked horizontal position on the toroidal map.
        y: Self-tracked vertical position on the toroidal map.
        orientation: Direction the player is currently facing.
        map_width: Width of the world in tiles (received at handshake).
        map_height: Height of the world in tiles (received at handshake).
        team_name: Name of the team this player belongs to.
        team_slots: Number of available connection slots for the team.
        fsm_state: Current FSM state; governs what the player does this tick.
        last_look: Most recent parsed Look result (list of tile contents).
        pending_inventory: True while an Inventory command is in flight.
        pending_look: True while a Look command is in flight.
    """

    level: int
    map_width: int
    map_height: int
    team_name: str
    team_slots: int
    inventory: Inventory = field(default_factory=empty_inventory)
    x: int = 0
    y: int = 0
    orientation: Orientation = Orientation.NORTH
    fsm_state: FsmState = FsmState.SCOUTING
    last_look: list = field(default_factory=list)
    pending_inventory: bool = False
    pending_look: bool = False


def make_player_state(team_name: str, team_slots: int,
                      map_width: int, map_height: int) -> PlayerState:
    """Construct the initial PlayerState after a successful handshake.

    The player starts at level 1, position (0, 0), facing North,
    with all inventory resources at zero.

    Args:
        team_name: Name of the team this player belongs to.
        team_slots: Number of available connection slots reported by the server.
        map_width: Width of the world in tiles.
        map_height: Height of the world in tiles.

    Returns:
        A freshly initialised PlayerState.
    """
    return PlayerState(
        level=1,
        map_width=map_width,
        map_height=map_height,
        team_name=team_name,
        team_slots=team_slots,
    )


def rotate_right(orientation: Orientation) -> Orientation:
    """Return the orientation after a 90-degree clockwise turn.

    Args:
        orientation: Current facing direction.

    Returns:
        New facing direction after turning right.
    """
    return Orientation((orientation.value + 1) % 4)


def rotate_left(orientation: Orientation) -> Orientation:
    """Return the orientation after a 90-degree counter-clockwise turn.

    Args:
        orientation: Current facing direction.

    Returns:
        New facing direction after turning left.
    """
    return Orientation((orientation.value - 1) % 4)


# Displacement vectors for each orientation: (dx, dy).
# North decrements y (row index decreases going forward),
# East increments x, South increments y, West decrements x.
_FORWARD_DELTA: dict[Orientation, tuple[int, int]] = {
    Orientation.NORTH: (0, -1),
    Orientation.EAST:  (1,  0),
    Orientation.SOUTH: (0,  1),
    Orientation.WEST:  (-1, 0),
}


def apply_forward(state: PlayerState) -> None:
    """Update the player's position after a successful Forward command.

    Moves one tile in the current orientation direction, wrapping
    coordinates toroidally using the stored map dimensions.

    Args:
        state: Mutable player state to update in place.
    """
    dx, dy = _FORWARD_DELTA[state.orientation]
    state.x = torus_wrap(state.x + dx, state.map_width)
    state.y = torus_wrap(state.y + dy, state.map_height)


def apply_right(state: PlayerState) -> None:
    """Update the player's orientation after a successful Right command.

    Args:
        state: Mutable player state to update in place.
    """
    state.orientation = rotate_right(state.orientation)


def apply_left(state: PlayerState) -> None:
    """Update the player's orientation after a successful Left command.

    Args:
        state: Mutable player state to update in place.
    """
    state.orientation = rotate_left(state.orientation)
