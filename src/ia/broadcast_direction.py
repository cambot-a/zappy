##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Broadcast direction-to-heading decoder
##

from player_state import Orientation

# Mapping from broadcast direction K (1-8) to the number of clockwise
# right turns needed to face toward the sound source.
#
# K encodes a clockwise sector from the receiver's forward direction:
#   K=1:  0  deg (front)
#   K=2:  45 deg (front-right)  -> snap to right  (1 turn)
#   K=3:  90 deg (right)        -> 1 right turn
#   K=4: 135 deg (back-right)   -> snap to back    (2 turns)
#   K=5: 180 deg (back)         -> 2 right turns
#   K=6: 225 deg (back-left)    -> snap to left    (3 turns)
#   K=7: 270 deg (left)         -> 3 right turns
#   K=8: 315 deg (front-left)   -> snap to front   (0 turns)
#
# Diagonal sectors (K=2,4,6,8) are rounded to the nearest cardinal.
# On ties (equal distance to two cardinals) the convention is:
#   K=2 -> right, K=4 -> back, K=6 -> left, K=8 -> front.
_K_TO_RIGHT_TURNS: dict[int, int] = {
    1: 0,
    2: 1,
    3: 1,
    4: 2,
    5: 2,
    6: 3,
    7: 3,
    8: 0,
}


def direction_to_orientation(k: int,
                              current: Orientation) -> Orientation | None:
    """Return the orientation to face in order to move toward a broadcast source.

    Given the broadcast direction indicator K (1-8) and the receiver's
    current facing, compute the target cardinal orientation that best
    approximates moving toward the sender.

    K=0 (same tile) has no meaningful direction; the function returns
    None in that case and the caller should not move.

    Args:
        k: Broadcast direction (0 = same tile, 1-8 clockwise from front).
        current: The receiver's current facing orientation.

    Returns:
        The target Orientation to face, or None if k == 0 or k is
        outside the valid range 0-8.
    """
    if k == 0:
        return None
    right_turns = _K_TO_RIGHT_TURNS.get(k)
    if right_turns is None:
        return None
    return Orientation((current.value + right_turns) % 4)


def is_approaching(k: int) -> bool:
    """Return True when the broadcast direction indicates the sender is near.

    A K of 1 means the sender is directly in front, which combined with
    close proximity is the signal that the follower is nearly at the
    leader's tile. K=0 means same tile.

    Args:
        k: Broadcast direction (0-8).

    Returns:
        True if k is 0 (same tile) or 1 (directly ahead).
    """
    return k in (0, 1)
