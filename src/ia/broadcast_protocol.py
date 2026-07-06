##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Structured broadcast protocol shared by all AI team members
##

from dataclasses import dataclass

# Message type constants.
# HAVE   - announce current level and stone counts to the team.
# RALLY  - leader invites followers to a tile for a joint incantation.
# COMING - follower acknowledges a RALLY and is heading to the tile.
# READY  - follower has arrived on the leader's tile.
# CANCEL - leader aborts the current rally (bad conditions).
MSG_HAVE = "HAVE"
MSG_RALLY = "RALLY"
MSG_COMING = "COMING"
MSG_READY = "READY"
MSG_CANCEL = "CANCEL"

_KNOWN_TYPES = frozenset({MSG_HAVE, MSG_RALLY, MSG_COMING, MSG_READY, MSG_CANCEL})

# Separator used in the broadcast text.
_SEP = ":"


@dataclass(frozen=True, slots=True)
class CoordMessage:
    """One parsed coordination message from a broadcast.

    Attributes:
        msg_type: One of the MSG_* constants.
        fields: Ordered list of string fields following the type tag.
    """

    msg_type: str
    fields: tuple[str, ...]


def format_msg(msg_type: str, *fields: str) -> str:
    """Format a coordination message for sending via Broadcast.

    Args:
        msg_type: One of the MSG_* constants.
        *fields: Additional string fields to include after the type.

    Returns:
        A string suitable for use as the text argument of a
        ``Broadcast <text>`` command.

    Example:
        >>> format_msg(MSG_HAVE, "1", "3", "0", "0", "0", "0", "0")
        'HAVE:1:3:0:0:0:0:0'
    """
    if fields:
        return _SEP.join((msg_type, *fields))
    return msg_type


def parse_msg(text: str) -> CoordMessage | None:
    """Parse the text portion of a received broadcast line.

    Only messages whose type tag is one of the known MSG_* constants are
    accepted. Unknown or malformed messages return None so the caller can
    silently ignore them.

    Args:
        text: The bare text extracted from ``message K, <text>``.

    Returns:
        CoordMessage on success, None if the text is not a valid
        coordination message.
    """
    parts = text.split(_SEP)
    msg_type = parts[0]
    if msg_type not in _KNOWN_TYPES:
        return None
    return CoordMessage(msg_type=msg_type, fields=tuple(parts[1:]))
