#!/usr/bin/env python3

##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## main
##

import signal
import sys
from connection import open_connection
from event_loop import run
from fsm import Fsm
from handshake import perform_handshake
from line_buffer import LineBuffer
from player_state import make_player_state
from sender import CommandSender

USAGE = "USAGE: ./zappy_ai -p port -n name [-h machine]"
ERROR = 84


def _die(message: str, show_usage: bool = True) -> None:
    """Print an error message to stderr and exit with code 84.

    Args:
        message: description of the error.
        show_usage: Whether to also print the usage line.
    """
    print(f"error: {message}", file=sys.stderr)
    if show_usage:
        print(USAGE, file=sys.stderr)
    sys.exit(ERROR)


def _parse_port(value: str, already_set: bool) -> int:
    """Validate and convert a raw port string to an integer.

    Args:
        value: Raw string value provided after the -p flag.
        already_set: True if -p was already encountered earlier.

    Returns:
        The validated port as an integer (1-65535).
    """
    if already_set:
        _die("duplicate flag -p", show_usage=False)
    try:
        port = int(value)
    except ValueError:
        _die(f"port must be an integer, got '{value}'", show_usage=False)
    if not (1 <= port <= 65535):
        _die(f"port must be in range 1-65535, got {port}", show_usage=False)
    return port


def _parse_name(value: str, already_set: bool) -> str:
    """Validate a team name string.

    Args:
        value: Raw string value provided after the -n flag.
        already_set: True if -n was already encountered earlier.

    Returns:
        The validated team name.
    """
    if already_set:
        _die("duplicate flag -n", show_usage=False)
    if not value:
        _die("team name must not be empty", show_usage=False)
    return value


def _consume_flag(argv: list[str], i: int) -> tuple[str, str]:
    """Read a flag and its required value from argv at position i.

    Args:
        argv: Full argument list.
        i: Index of the flag token.

    Returns:
        (flag, value) pair.
    """
    flag = argv[i]
    if i + 1 >= len(argv):
        _die(f"flag {flag} requires an argument")
    return flag, argv[i + 1]


def _check_required(port: int | None, name: str | None) -> None:
    """Ensure that all required flags were provided.

    Args:
        port: Parsed port value, or None if -p was absent.
        name: Parsed team name, or None if -n was absent.
    """
    if port is None:
        _die("missing required flag -p")
    if name is None:
        _die("missing required flag -n")


def parse_args(argv: list[str]) -> tuple[int, str, str]:
    """Parse and validate the full argument list.

    Args:
        argv: Argument list excluding the program name (sys.argv[1:]).

    Returns:
        (port, name, machine) with machine defaulting to 'localhost'.
    """
    if "--help" in argv:
        print(USAGE, file=sys.stderr)
        sys.exit(0)

    port: int | None = None
    name: str | None = None
    machine: str = "localhost"

    i = 0
    while i < len(argv):
        if argv[i] not in ("-p", "-n", "-h"):
            _die(f"unknown argument '{argv[i]}'")
        flag, value = _consume_flag(argv, i)
        i += 2
        if flag == "-p":
            port = _parse_port(value, port is not None)
        elif flag == "-n":
            name = _parse_name(value, name is not None)
        elif flag == "-h":
            machine = value

    _check_required(port, name)
    return port, name, machine


if __name__ == "__main__":
    signal.signal(signal.SIGINT, lambda _s, _f: sys.exit(0))
    signal.signal(signal.SIGTERM, lambda _s, _f: sys.exit(0))
    port, name, machine = parse_args(sys.argv[1:])
    sock = open_connection(machine, port)
    info = perform_handshake(sock, name)
    print(f"connected to {machine}:{port} | slots={info.slots} map={info.width}x{info.height}",
          file=sys.stderr)
    buf = LineBuffer(sock)
    sender = CommandSender(sock)
    state = make_player_state(name, info.slots, info.width, info.height)
    fsm = Fsm(state)
    run(buf, sender, fsm_tick=fsm.tick, on_broadcast=fsm.on_broadcast)
