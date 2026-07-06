##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Line-buffered socket reader
##

import socket
import sys

ERROR = 84
_CHUNK_SIZE = 4096


class LineBuffer:
    """Accumulates raw bytes from a socket and yields complete lines.

    Bytes are read from the socket in chunks. Complete newline-terminated
    lines are extracted and returned one at a time. Incomplete trailing
    data is held in the internal buffer until the next read fills it.

    This is the single place in the AI where raw socket I/O happens
    during the main event loop. The handshake uses its own simpler reader.

    Attributes:
        _sock: The connected TCP socket to read from.
        _buf: Internal byte accumulator.
    """

    def __init__(self, sock: socket.socket) -> None:
        """Initialise the buffer with a connected socket.

        Args:
            sock: A connected, blocking TCP socket.
        """
        self._sock = sock
        self._buf = b""

    def _read_chunk(self) -> bool:
        """Read one chunk of bytes from the socket into the buffer.

        Returns:
            True if data was received, False if the connection was closed.
        """
        try:
            chunk = self._sock.recv(_CHUNK_SIZE)
        except OSError as exc:
            print(f"error: socket read failed: {exc}", file=sys.stderr)
            sys.exit(ERROR)
        if not chunk:
            return False
        self._buf += chunk
        return True

    def _extract_line(self) -> str | None:
        """Extract and return the first complete line from the buffer.

        Returns:
            The line as a string (without the trailing newline), or None
            if no complete line is currently in the buffer.
        """
        idx = self._buf.find(b"\n")
        if idx == -1:
            return None
        line = self._buf[:idx].decode(errors="replace")
        self._buf = self._buf[idx + 1:]
        return line

    def readline(self) -> str | None:
        """Return the next complete line from the socket.

        Reads chunks from the socket until a newline-terminated line is
        available in the internal buffer. This guarantees that fragmented
        TCP delivery (data arriving in very small chunks) is handled
        correctly: the method only returns None when the server closes the
        connection, never because a partial line has not yet arrived.

        Returns:
            The next line as a string (without the trailing newline), or
            None if the server closed the connection.
        """
        while True:
            line = self._extract_line()
            if line is not None:
                return line
            if not self._read_chunk():
                return None

    def readlines(self) -> list[str]:
        """Return all complete lines currently available in the buffer.

        Reads one chunk from the socket, then drains every complete line
        that was produced. Suitable for non-blocking polling in the main
        event loop: call once per iteration to collect all pending server
        messages.

        Returns:
            A list of complete lines (possibly empty if no full line
            arrived yet). Returns an empty list if the connection closed
            mid-read, after which the caller should treat the session
            as terminated.
        """
        if not self._read_chunk():
            return []
        lines: list[str] = []
        while True:
            line = self._extract_line()
            if line is None:
                break
            lines.append(line)
        return lines
