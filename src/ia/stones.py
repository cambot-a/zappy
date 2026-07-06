##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Needed stones calculator for elevation rituals
##

from inventory import Inventory

# Elevation requirement table.
# Key: current level (1-7).
# Value: dict mapping resource name to quantity required on the tile.
# "players" is the required number of players (including the initiator).
ELEVATION_REQUIREMENTS: dict[int, dict[str, int]] = {
    1: {"players": 1, "linemate": 1, "deraumere": 0, "sibur": 0,
        "mendiane": 0, "phiras": 0, "thystame": 0},
    2: {"players": 2, "linemate": 1, "deraumere": 1, "sibur": 1,
        "mendiane": 0, "phiras": 0, "thystame": 0},
    3: {"players": 2, "linemate": 2, "deraumere": 0, "sibur": 1,
        "mendiane": 0, "phiras": 2, "thystame": 0},
    4: {"players": 4, "linemate": 1, "deraumere": 1, "sibur": 2,
        "mendiane": 0, "phiras": 1, "thystame": 0},
    5: {"players": 4, "linemate": 1, "deraumere": 2, "sibur": 1,
        "mendiane": 3, "phiras": 0, "thystame": 0},
    6: {"players": 6, "linemate": 1, "deraumere": 2, "sibur": 3,
        "mendiane": 0, "phiras": 1, "thystame": 0},
    7: {"players": 6, "linemate": 2, "deraumere": 2, "sibur": 2,
        "mendiane": 2, "phiras": 2, "thystame": 1},
}

# Stone resource names in canonical order (food excluded).
STONE_NAMES = ("linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame")


def needed_stones(level: int, inventory: Inventory) -> dict[str, int]:
    """Compute which stones the player still needs to collect for its ritual.

    Compares the player's current inventory against the elevation
    requirement for the given level. Returns only entries with a positive
    delta (still needed). Food is not included.

    Args:
        level: Current player level (1-7). Level 8 needs nothing.
        inventory: Player's current resource counts.

    Returns:
        Dictionary mapping resource name to the quantity still needed.
        An empty dict means the player has everything required.
    """
    if level not in ELEVATION_REQUIREMENTS:
        return {}
    req = ELEVATION_REQUIREMENTS[level]
    result: dict[str, int] = {}
    for stone in STONE_NAMES:
        needed = req.get(stone, 0) - inventory.get(stone, 0)
        if needed > 0:
            result[stone] = needed
    return result


def has_all_stones(level: int, inventory: Inventory) -> bool:
    """Return True if the player already holds all stones for its ritual.

    Args:
        level: Current player level.
        inventory: Player's current resource counts.

    Returns:
        True when needed_stones returns an empty dict.
    """
    return len(needed_stones(level, inventory)) == 0


def required_players(level: int) -> int:
    """Return the number of same-level players needed for the ritual.

    Args:
        level: Current player level (1-7).

    Returns:
        Required player count (including the initiator), or 0 if level
        is out of range.
    """
    req = ELEVATION_REQUIREMENTS.get(level)
    return req["players"] if req else 0
