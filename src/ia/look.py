##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Look response parser
##

from inventory import RESOURCES

# Objects that can appear as tokens inside a tile description.
# "player" is not a resource but is a valid tile token.
_VALID_TOKENS = frozenset(RESOURCES) | {"player"}

# Type alias: a tile is a list of token strings (resource names / "player").
Tile = list[str]
LookResult = list[Tile]


def _expected_tile_count(level: int) -> int:
    """Return the number of tiles visible at a given player level.

    The visible area forms a triangle: at level L the player sees
    L rows in front plus the current tile, giving (L+1)^2 tiles total.

    Args:
        level: Current player level (1-8).

    Returns:
        Total number of tiles in the Look response at that level.
    """
    return (level + 1) ** 2


def _strip_brackets(line: str) -> str | None:
    """Remove surrounding brackets from a Look response line.

    Args:
        line: Raw server line, e.g. '[player, food linemate, ]'.

    Returns:
        Inner content string, or None if brackets are absent.
    """
    stripped = line.strip()
    if not (stripped.startswith("[") and stripped.endswith("]")):
        return None
    return stripped[1:-1]


def _parse_tile(raw: str) -> Tile:
    """Parse a single tile string into a list of tokens.

    Each token is a space-separated item name within the tile. Empty
    tiles (no items) produce an empty list.

    Args:
        raw: A single tile string such as 'player food linemate'.

    Returns:
        List of item name strings on that tile.
    """
    tokens = raw.strip().split()
    return [t for t in tokens if t in _VALID_TOKENS]


def tile_index_to_offset(index: int, level: int) -> tuple[int, int]:
    """Convert a Look tile index to a local (dx, dy) offset.

    Tile 0 is the player's own tile. Subsequent tiles are numbered
    left-to-right, row by row, starting from the row directly in front.
    The player faces North (dy = -1 direction in local frame).

    Coordinate convention:
        dx > 0 means to the right of the player.
        dy > 0 means in front of the player (North in local frame).

    Args:
        index: Tile index from the Look response (0 to (level+1)^2 - 1).
        level: Current player level (1-8).

    Returns:
        (dx, dy) offset relative to the player's current position in the
        player's local frame (before applying world orientation).

    Raises:
        ValueError: If index is out of range for the given level.
    """
    total = _expected_tile_count(level)
    if not (0 <= index < total):
        raise ValueError(f"tile index {index} out of range for level {level}")
    if index == 0:
        return (0, 0)

    # Walk through rows starting at distance 1 in front.
    tile_num = 1
    for row in range(1, level + 1):
        row_width = 2 * row + 1
        if index < tile_num + row_width:
            col = index - tile_num
            dx = col - row   # centre column of this row has dx=0
            dy = row
            return (dx, dy)
        tile_num += row_width
    raise ValueError(f"tile index {index} could not be mapped at level {level}")


def parse_look(line: str, level: int) -> LookResult | None:
    """Parse a server Look response into a list of tile contents.

    Each element of the returned list corresponds to one tile in the
    Look field of view. The tile at index 0 is the player's own tile.
    Subsequent tiles follow the numbering described in the subject.

    The parser is lenient about unknown tokens: they are silently
    dropped so that a new server version with extra objects does not
    crash the client.

    Returns None (rather than raising) on structural errors so the
    caller can decide how to handle a malformed response.

    Args:
        line: Raw server response line starting with '[' and ending
              with ']', containing comma-separated tile descriptions.
        level: Current player level, used to validate the tile count.

    Returns:
        List of Tile lists (one per tile), or None if the line does not
        have the expected structure or tile count.
    """
    inner = _strip_brackets(line)
    if inner is None:
        return None

    raw_tiles = inner.split(",")
    expected = _expected_tile_count(level)
    if len(raw_tiles) != expected:
        return None

    return [_parse_tile(raw) for raw in raw_tiles]
