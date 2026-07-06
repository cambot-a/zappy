##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## HAVE broadcast helpers and team member state tracker
##

from dataclasses import dataclass

from broadcast_protocol import CoordMessage, MSG_HAVE, format_msg, parse_msg
from player_state import PlayerState
from sender import CommandSender

# Number of FSM ticks between HAVE broadcasts.
HAVE_BROADCAST_INTERVAL = 40

# Stone field order inside a HAVE message (must stay consistent).
_HAVE_STONE_ORDER = ("linemate", "deraumere", "sibur", "mendiane",
                     "phiras", "thystame")


@dataclass(slots=True)
class TeamMemberState:
    """Last known state of one teammate parsed from HAVE broadcasts.

    Attributes:
        level: Reported level.
        stones: Stone counts keyed by name.
        direction: Last broadcast K direction (0 = same tile).
    """

    level: int
    stones: dict[str, int]
    direction: int


def format_have(state: PlayerState) -> str:
    """Format a HAVE broadcast text for the current player state."""
    fields = [str(state.level)]
    for stone in _HAVE_STONE_ORDER:
        fields.append(str(state.inventory.get(stone, 0)))
    return format_msg(MSG_HAVE, *fields)


def parse_have(msg: CoordMessage) -> TeamMemberState | None:
    """Parse a HAVE CoordMessage into a TeamMemberState.

    Returns None if the message is malformed (wrong field count, non-integer
    values, or level < 1).
    """
    if len(msg.fields) != 7:
        return None
    try:
        values = [int(f) for f in msg.fields]
    except ValueError:
        return None
    if values[0] < 1:
        return None
    stones = {name: values[i + 1] for i, name in enumerate(_HAVE_STONE_ORDER)}
    return TeamMemberState(level=values[0], stones=stones, direction=0)


def on_broadcast_received(
        team_members: dict[int, TeamMemberState],
        direction: int,
        text: str) -> None:
    """Update the team member table from a received broadcast.

    Handles HAVE messages only. Silently ignores unknown or non-HAVE
    messages. The table is keyed by the broadcast K direction value.
    """
    msg = parse_msg(text)
    if msg is None:
        return
    if msg.msg_type == MSG_HAVE:
        member = parse_have(msg)
        if member is not None:
            member.direction = direction
            team_members[direction] = member


def broadcast_have(state: PlayerState, sender: CommandSender,
                   tick: int) -> bool:
    """Emit a HAVE broadcast if the broadcast interval has elapsed.

    Does nothing if the sender pipeline is full. Returns True if sent.
    """
    if tick % HAVE_BROADCAST_INTERVAL != 0:
        return False
    if not sender.can_send:
        return False
    sender.send(f"Broadcast {format_have(state)}", callback=lambda r: None)
    return True
