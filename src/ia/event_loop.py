##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Main event loop
##

import sys
from typing import Callable

from broadcast import parse_broadcast
from dispatcher import Dispatcher
from elevation import (
    ElevationState,
    make_current_level_handler,
    make_elevation_underway_handler,
    make_ko_elevation_handler,
)
from line_buffer import LineBuffer
from sender import CommandSender

FsmTickFn = Callable[[CommandSender, ElevationState], None]
BroadcastFn = Callable[[CommandSender, int, str, ElevationState], None]


def _handle_dead(_line: str) -> None:
    """Handle the 'dead' message from the server.

    The server sends 'dead' when the player has run out of food.
    The process must terminate immediately.

    Args:
        _line: The full line (always 'dead'), ignored.
    """
    sys.exit(0)


def _on_connection_closed() -> None:
    """Called when the server closes the TCP connection unexpectedly.

    Prints a notice to stderr and exits cleanly.
    """
    print("connection closed by server", file=sys.stderr)
    sys.exit(0)


def _build_dispatcher(sender: CommandSender,
                      elev: ElevationState,
                      on_broadcast: "BroadcastFn | None" = None) -> Dispatcher:
    """Construct the dispatcher and register built-in handlers.

    Registered handlers (in priority order):
        - 'dead'              : exit(0) immediately.
        - 'Elevation underway': mark elevation as IN_PROGRESS.
        - 'Current level: '  : parse new level, mark SUCCESS.
        - 'message '         : parse broadcast and call on_broadcast (optional).
    All other lines fall through to sender.on_response.

    Args:
        sender: The command sender whose on_response acts as the fallback.
        elev: Shared elevation state updated by the elevation handlers.
        on_broadcast: Optional callable invoked for each received broadcast
            with (sender, direction, text).

    Returns:
        A configured Dispatcher instance.
    """
    d = Dispatcher(fallback=sender.on_response)
    d.register("dead", _handle_dead)
    d.register("Elevation underway", make_elevation_underway_handler(elev, sender))
    d.register("Current level: ", make_current_level_handler(elev))
    d.register("ko", make_ko_elevation_handler(elev, sender))
    if on_broadcast is not None:
        def _bcast_handler(line: str) -> None:
            msg = parse_broadcast(line)
            if msg is not None:
                on_broadcast(sender, msg.direction, msg.text, elev)
        d.register("message ", _bcast_handler)
    return d


def run(buf: LineBuffer, sender: CommandSender,
        fsm_tick: FsmTickFn,
        on_broadcast: "BroadcastFn | None" = None) -> None:
    """Run the main event loop until the session ends.

    Each iteration:
        1. Block until the next complete line arrives from the server.
        2. Dispatch the line to its registered handler.
        3. Call fsm_tick so the FSM can evaluate its state and send commands.

    The loop exits (via sys.exit) on:
        - Receipt of 'dead' from the server.
        - The server closing the TCP connection.

    Args:
        buf: Line-buffered socket reader.
        sender: Command sender with in-flight tracking.
        fsm_tick: Callable invoked once per loop iteration.
        on_broadcast: Optional callable for received broadcast messages.
    """
    elev = ElevationState()
    dispatcher = _build_dispatcher(sender, elev, on_broadcast)

    fsm_tick(sender, elev)

    while True:
        line = buf.readline()
        if line is None:
            _on_connection_closed()
            return
        dispatcher.dispatch(line)
        fsm_tick(sender, elev)
