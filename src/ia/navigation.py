##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Navigation: turn resolver and move-to-tile routine
##

from player_state import Orientation, PlayerState, apply_forward, apply_left, apply_right
from sender import CommandSender
from torus import torus_distance


def turns_to_face(current: Orientation, target: Orientation) -> list[str]:
    """Compute the minimal sequence of turn commands to face a target orientation.

    At most two turns are ever needed. If both a single-right and a
    single-left would reach the target in the same number of steps,
    right is preferred.

    Args:
        current: The player's current facing direction.
        target: The desired facing direction.

    Returns:
        A list of 'Right' and/or 'Left' command strings (0, 1, or 2 items).
    """
    if current == target:
        return []
    cw = (target.value - current.value) % 4   # steps clockwise
    ccw = (current.value - target.value) % 4  # steps counter-clockwise
    if cw <= ccw:
        return ["Right"] * cw
    return ["Left"] * ccw


def _orientation_from_delta(dx: int, dy: int) -> Orientation | None:
    """Return the orientation that matches a unit displacement vector.

    Only the four cardinal directions are supported. Returns None for
    diagonal or zero vectors, which the caller must handle.

    Args:
        dx: Horizontal component (-1, 0, or 1).
        dy: Vertical component (-1, 0, or 1).

    Returns:
        The matching Orientation, or None if the vector is not cardinal.
    """
    mapping = {
        (0, -1): Orientation.NORTH,
        (1, 0): Orientation.EAST,
        (0, 1): Orientation.SOUTH,
        (-1, 0): Orientation.WEST,
    }
    return mapping.get((dx, dy))


def _steps_to_reach(sx: int, sy: int, tx: int, ty: int,
                    width: int, height: int) -> list[tuple[Orientation, int]]:
    """Break the path from (sx, sy) to (tx, ty) into axis-aligned segments.

    Each segment is a (orientation, steps) pair. At most two segments
    are returned (one horizontal, one vertical). The shortest toroidal
    path is always used on each axis. Segments with zero steps are
    omitted.

    Args:
        sx: Source x coordinate.
        sy: Source y coordinate.
        tx: Target x coordinate.
        ty: Target y coordinate.
        width: Map width for toroidal arithmetic.
        height: Map height for toroidal arithmetic.

    Returns:
        List of (Orientation, steps) pairs to enqueue in order.
    """
    segments: list[tuple[Orientation, int]] = []
    dx = torus_distance(sx, tx, width)
    dy = torus_distance(sy, ty, height)

    if dx != 0:
        orient = Orientation.EAST if dx > 0 else Orientation.WEST
        segments.append((orient, abs(dx)))
    if dy != 0:
        orient = Orientation.SOUTH if dy > 0 else Orientation.NORTH
        segments.append((orient, abs(dy)))
    return segments


def navigate_to(state: PlayerState, sender: CommandSender,
                tx: int, ty: int) -> int:
    """Enqueue the turn and forward commands needed to reach tile (tx, ty).

    Commands are enqueued up to the sender's available capacity. If the
    full path does not fit in the current pipeline slot budget, as many
    commands as possible are enqueued and the function returns the number
    of commands actually sent. The caller should invoke this again on the
    next FSM tick until it returns 0 (destination reached or pipeline full).

    Dead reckoning (apply_forward / apply_right / apply_left) is applied
    immediately to the player state for each command queued, so the state
    always reflects the expected position after all queued commands execute.

    Args:
        state: Mutable player state (position and orientation updated here).
        sender: Command sender with in-flight tracking.
        tx: Target x coordinate (toroidal).
        ty: Target y coordinate (toroidal).

    Returns:
        Number of commands enqueued (0 means already at the destination
        or the pipeline is full).
    """
    sent = 0
    segments = _steps_to_reach(state.x, state.y, tx, ty,
                                state.map_width, state.map_height)

    for orient, steps in segments:
        turn_cmds = turns_to_face(state.orientation, orient)
        for cmd in turn_cmds:
            if not sender.can_send:
                return sent
            if cmd == "Right":
                sender.send("Right")
                apply_right(state)
            else:
                sender.send("Left")
                apply_left(state)
            sent += 1

        for _ in range(steps):
            if not sender.can_send:
                return sent
            sender.send("Forward")
            apply_forward(state)
            sent += 1

    return sent
