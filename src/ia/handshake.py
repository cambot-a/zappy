##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Server handshake sequence
##

import socket
import sys
from dataclasses import dataclass

ERROR = 84

@dataclass
class ServerInfo:
    """Information received from the server during the handshake.

    Attributes:
        slots: Number of available connection slots for the team.
        width: Width of the world map in tiles.
        height: Height of the world map in tiles.
    """

    slots: int
    width: int
    height: int


def _recv_line(sock: socket.socket) -> str:
    """Read bytes from the socket until a newline is encountered.

    This is a simple sequential reader used only during the handshake,
    before the main event loop and its proper line-buffered reader are
    in place.

    Args:
        sock: Connected TCP socket.

    Returns:
        The received line stripped of its trailing newline.
    """
    data = b""
    while not data.endswith(b"\n"):
        try:
            chunk = sock.recv(1)
        except OSError as exc:
            print(f"error: lost connection during handshake: {exc}",
                  file=sys.stderr)
            sys.exit(ERROR)
        if not chunk:
            print("error: server closed connection during handshake",
                  file=sys.stderr)
            sys.exit(ERROR)
        data += chunk
    return data.decode().rstrip("\n")


def _send_line(sock: socket.socket, text: str) -> None:
    """Send a single line to the server, terminated by a newline.

    Args:
        sock: Connected TCP socket.
        text: Text to send (without the trailing newline).
    """
    try:
        sock.sendall((text + "\n").encode())
    except OSError as exc:
        print(f"error: cannot send to server: {exc}", file=sys.stderr)
        sys.exit(ERROR)


def _expect_welcome(sock: socket.socket) -> None:
    """Read the opening WELCOME message from the server.

    Exits with code 84 if the first line is not 'WELCOME'.

    Args:
        sock: Connected TCP socket.
    """
    line = _recv_line(sock)
    if line != "WELCOME":
        print(f"error: expected 'WELCOME', got '{line}'", file=sys.stderr)
        sys.exit(ERROR)


def _send_team_name(sock: socket.socket, name: str) -> None:
    """Send the team name to the server.

    Args:
        sock: Connected TCP socket.
        name: Team name string (must not be 'GRAPHIC').
    """
    _send_line(sock, name)


def _recv_slots(sock: socket.socket) -> int:
    """Read the CLIENT-NUM line and return the number of available slots.

    Exits with code 84 if the line is not a valid integer.

    Args:
        sock: Connected TCP socket.

    Returns:
        Number of available slots for the team (>= 0).
    """
    line = _recv_line(sock)
    try:
        slots = int(line)
    except ValueError:
        print(f"error: expected CLIENT-NUM integer, got '{line}'",
              file=sys.stderr)
        sys.exit(ERROR)
    return slots


def _recv_map_size(sock: socket.socket) -> tuple[int, int]:
    """Read the 'X Y' map dimensions line.

    Exits with code 84 if the line does not contain two positive integers.

    Args:
        sock: Connected TCP socket.

    Returns:
        (width, height) in tiles.
    """
    line = _recv_line(sock)
    parts = line.split()
    if len(parts) != 2:
        print(f"error: expected 'X Y' map size, got '{line}'", file=sys.stderr)
        sys.exit(ERROR)
    try:
        width, height = int(parts[0]), int(parts[1])
    except ValueError:
        print(f"error: map size values must be integers, got '{line}'",
              file=sys.stderr)
        sys.exit(ERROR)
    if width <= 0 or height <= 0:
        print(f"error: map size must be positive, got {width}x{height}",
              file=sys.stderr)
        sys.exit(ERROR)
    return width, height


def perform_handshake(sock: socket.socket, team_name: str) -> ServerInfo:
    """Execute the full connection handshake with the server.

    Sequence:
        1. Receive 'WELCOME\\n' from the server.
        2. Send team name followed by '\\n'.
        3. Receive CLIENT-NUM (available slots for the team).
        4. Receive 'X Y' (map dimensions).

    Args:
        sock: Connected TCP socket.
        team_name: Name of the team this client belongs to.

    Returns:
        A ServerInfo dataclass with slots, width, and height.
    """
    _expect_welcome(sock)
    _send_team_name(sock, team_name)
    slots = _recv_slots(sock)
    width, height = _recv_map_size(sock)
    return ServerInfo(slots=slots, width=width, height=height)
