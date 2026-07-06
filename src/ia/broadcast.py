##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Broadcast message receiver and parser
##

from dataclasses import dataclass


@dataclass(frozen=True)
class BroadcastMessage:
    """A parsed incoming broadcast from another player.

    Attributes:
        direction: Tile number (0-8) indicating where the sound comes from.
            0 means the sender is on the same tile as the receiver.
            1 is directly in front; 2-8 continue clockwise around the receiver.
        text: The raw text content of the broadcast message.
    """

    direction: int
    text: str


def _parse_direction(raw: str) -> int | None:
    """Parse and validate the direction integer from a broadcast line.

    Args:
        raw: The raw direction token (should be '0' through '8').

    Returns:
        Integer direction (0-8), or None if the value is invalid.
    """
    try:
        k = int(raw)
    except ValueError:
        return None
    if not (0 <= k <= 8):
        return None
    return k


def parse_broadcast(line: str) -> BroadcastMessage | None:
    """Parse a server broadcast notification line.

    Expected format: 'message K, text'
    where K is an integer in 0-8 and text is any string.

    The text may itself contain commas; only the first ', ' after 'message '
    is treated as the separator between K and text.

    Args:
        line: Raw server line (without the trailing newline).

    Returns:
        A BroadcastMessage dataclass, or None if the line does not match
        the expected format.
    """
    if not line.startswith("message "):
        return None
    body = line[len("message "):]
    sep = body.find(", ")
    if sep == -1:
        return None
    raw_k = body[:sep]
    text = body[sep + 2:]
    direction = _parse_direction(raw_k)
    if direction is None:
        return None
    return BroadcastMessage(direction=direction, text=text)
