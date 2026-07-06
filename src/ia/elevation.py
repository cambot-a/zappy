##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Elevation response parsers and state tracking
##

from dataclasses import dataclass, field
from enum import Enum, auto


class ElevationStatus(Enum):
    """Possible states of an in-progress or recently completed elevation.

    Attributes:
        IDLE: No incantation is in progress.
        IN_PROGRESS: 'Elevation underway' was received; the player is frozen.
        SUCCESS: 'Current level: k' was received; the ritual succeeded.
        FAILED: 'ko' was received in the context of an incantation.
    """

    IDLE = auto()
    IN_PROGRESS = auto()
    SUCCESS = auto()
    FAILED = auto()


@dataclass
class ElevationState:
    """Mutable container for the current elevation tracking data.

    This object is shared between the parser callbacks and the FSM so
    that both can observe and update elevation progress.

    Attributes:
        status: Current elevation status.
        new_level: Level reached after a successful elevation (0 if none).
        incantation_sent: True while this player's Incantation command is in
            flight. Used by the 'Elevation underway' handler to determine
            whether to consume the pending callback slot (leaders only).
    """

    status: ElevationStatus = ElevationStatus.IDLE
    new_level: int = 0
    incantation_sent: bool = False


def parse_current_level(line: str) -> int | None:
    """Extract the level number from a 'Current level: k' server line.

    Args:
        line: Raw server response line.

    Returns:
        The new level as an integer, or None if the line does not match
        the expected format or the level is not a positive integer.
    """
    prefix = "Current level: "
    if not line.startswith(prefix):
        return None
    raw = line[len(prefix):]
    try:
        level = int(raw)
    except ValueError:
        return None
    if level < 1:
        return None
    return level


def is_elevation_underway(line: str) -> bool:
    """Return True if the line is the 'Elevation underway' server message.

    Args:
        line: Raw server response line.

    Returns:
        True when the line exactly matches 'Elevation underway'.
    """
    return line.strip() == "Elevation underway"


def make_elevation_underway_handler(state: ElevationState, sender=None):
    """Create a dispatcher handler that marks an elevation as in progress.

    When sender is provided and state.incantation_sent is True, this
    handler also consumes the pending Incantation callback slot so that
    the sender's in-flight counter stays accurate.  Only the player who
    actually sent Incantation (the leader) sets incantation_sent=True;
    followers receive 'Elevation underway' too but must not consume a
    slot they never allocated.

    Args:
        state: Shared ElevationState object to update.
        sender: Optional CommandSender. When provided and incantation_sent
            is True, consume_response is called to dequeue the slot.

    Returns:
        A handler callable suitable for Dispatcher.register.
    """
    def handler(line: str) -> None:
        state.status = ElevationStatus.IN_PROGRESS
        if sender is not None and state.incantation_sent:
            state.incantation_sent = False
            sender.consume_response(line)

    return handler


def make_current_level_handler(state: ElevationState):
    """Create a dispatcher handler for 'Current level: k' responses.

    Args:
        state: Shared ElevationState object to update.

    Returns:
        A handler callable suitable for Dispatcher.register.
    """
    def handler(line: str) -> None:
        """Parse the new level and mark the elevation as successful.

        Args:
            line: Full server line, e.g. 'Current level: 3'.
        """
        level = parse_current_level(line)
        if level is not None:
            state.new_level = level
            state.status = ElevationStatus.SUCCESS

    return handler


def make_ko_elevation_handler(state: ElevationState, sender=None):
    """Create a dispatcher handler for 'ko' during an in-progress elevation.

    When the incantation fails, the server sends 'ko' to all participants
    (leader and followers). The leader's 'ko' is consumed by sender.on_response
    as a reply to the Incantation command. But followers never sent Incantation,
    so their 'ko' arrives without a pending callback and would be ignored.

    This handler intercepts 'ko' when ElevationStatus is IN_PROGRESS and marks
    the elevation as FAILED so the FSM can exit the ELEVATING state.

    If the sender has a pending command in flight (leader case), we consume
    the response slot so the sender's in-flight count stays accurate.

    Args:
        state: Shared ElevationState object to update.
        sender: Optional CommandSender; if provided and has a pending response,
            the 'ko' is consumed as that response.

    Returns:
        A handler callable suitable for Dispatcher.register.
    """
    def handler(line: str) -> None:
        if state.status == ElevationStatus.IN_PROGRESS:
            state.status = ElevationStatus.FAILED
            if sender is not None and sender.in_flight > 0:
                sender.consume_response(line)
        elif sender is not None:
            sender.on_response(line)

    return handler
