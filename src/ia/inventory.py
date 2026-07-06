##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Inventory response parser
##

import sys

RESOURCES = ("food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame")

Inventory = dict[str, int]


def empty_inventory() -> Inventory:
    """Return an inventory with every resource set to zero.

    Returns:
        Dictionary mapping each resource name to 0.
    """
    return {r: 0 for r in RESOURCES}


def _strip_brackets(line: str) -> str | None:
    """Remove the surrounding brackets from an inventory line.

    Args:
        line: Raw line from the server, ex :. '[food 3, linemate 1, ...]'.

    Returns:
        The inner content string, or None if the brackets are missing.
    """
    stripped = line.strip()
    if not (stripped.startswith("[") and stripped.endswith("]")):
        return None
    return stripped[1:-1]


def _parse_entry(entry: str) -> tuple[str, int] | None:
    """Parse a single 'resource N' entry from the inventory line.

    Args:
        entry: A single comma-separated token, ex :. 'food 3'.

    Returns:
        (resource_name, quantity) or None if the entry is malformed.
    """
    parts = entry.strip().split()
    if len(parts) != 2:
        return None
    name, raw_count = parts
    if name not in RESOURCES:
        return None
    try:
        count = int(raw_count)
    except ValueError:
        return None
    if count < 0:
        return None
    return name, count


def parse_inventory(line: str) -> Inventory | None:
    """Parse a server inventory response line into a resource dictionary.

    Expected format: [food N, linemate N, deraumere N, sibur N,
                      mendiane N, phiras N, thystame N]

    The order of entries in the line is not enforced; any resource not
    present in the line defaults to 0. Returns None on any parse error
    so the caller can decide whether to log or ignore the malformed line.

    Args:
        line: Raw response line from the server.

    Returns:
        Dictionary mapping resource names to their quantities, or None
        if the line does not match the expected inventory format.
    """
    inner = _strip_brackets(line)
    if inner is None:
        return None

    result = empty_inventory()
    for entry in inner.split(","):
        if not entry.strip():
            continue
        parsed = _parse_entry(entry)
        if parsed is None:
            return None
        name, count = parsed
        result[name] = count

    return result
