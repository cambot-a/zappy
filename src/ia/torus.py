##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Toroidal coordinate arithmetic
##


def torus_wrap(value: int, size: int) -> int:
    """Wrap a coordinate into the range [0, size) on a toroidal axis.

    Args:
        value: Raw coordinate, possibly outside [0, size).
        size: Length of the axis (map width or height).

    Returns:
        The coordinate normalised into [0, size).
    """
    return value % size


def torus_distance(a: int, b: int, size: int) -> int:
    """Shortest signed distance from coordinate a to coordinate b on a
    toroidal axis of the given size.

    The result is in the range (-size/2, size/2]. A positive value means
    b is clockwise (or right/down) from a along the shorter path.

    Args:
        a: Source coordinate (already normalised).
        b: Target coordinate (already normalised).
        size: Length of the axis.

    Returns:
        Signed shortest displacement from a to b.
    """
    raw = (b - a) % size
    if raw > size // 2:
        raw -= size
    return raw


def torus_manhattan(ax: int, ay: int, bx: int, by: int,
                    width: int, height: int) -> int:
    """Manhattan distance between two points on a toroidal map.

    Args:
        ax: X coordinate of point A.
        ay: Y coordinate of point A.
        bx: X coordinate of point B.
        by: Y coordinate of point B.
        width: Map width in tiles.
        height: Map height in tiles.

    Returns:
        Sum of the shortest x-distance and shortest y-distance.
    """
    dx = abs(torus_distance(ax, bx, width))
    dy = abs(torus_distance(ay, by, height))
    return dx + dy
