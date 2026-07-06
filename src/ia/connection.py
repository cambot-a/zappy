##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## TCP connection to the zappy server
##

import socket
import sys

ERROR = 84


def _create_socket(family: int) -> socket.socket:
    """Create a TCP socket for the given address family.

    Args:
        family: Address family (socket.AF_INET or socket.AF_INET6).

    Returns:
        A new, unconnected socket.
    """
    try:
        sock = socket.socket(family, socket.SOCK_STREAM)
    except OSError as exc:
        print(f"error: cannot create socket: {exc}", file=sys.stderr)
        sys.exit(ERROR)
    return sock


def _connect_socket(sock: socket.socket, sockaddr: tuple) -> None:
    """Connect an already-created socket to the given address.

    Args:
        sock: The socket to connect.
        sockaddr: Address tuple returned by getaddrinfo.
    """
    try:
        sock.connect(sockaddr)
    except OSError as exc:
        print(f"error: cannot connect to server: {exc}", file=sys.stderr)
        sock.close()
        sys.exit(ERROR)


def open_connection(machine: str, port: int) -> socket.socket:
    """Open a blocking TCP connection to the zappy server.

    Resolves the hostname, creates the socket, and connects.
    Exits with code 84 on any network error.

    Args:
        machine: Hostname or IP address of the server.
        port: TCP port number (1-65535).

    Returns:
        A connected, blocking TCP socket.
    """
    try:
        info = socket.getaddrinfo(machine, port, type=socket.SOCK_STREAM)
    except socket.gaierror as exc:
        print(f"error: cannot resolve '{machine}': {exc}", file=sys.stderr)
        sys.exit(ERROR)
    if not info:
        print(f"error: no address found for '{machine}'", file=sys.stderr)
        sys.exit(ERROR)
    family, _, _, _, sockaddr = info[0]
    sock = _create_socket(family)
    _connect_socket(sock, sockaddr)
    return sock
