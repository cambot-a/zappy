##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## Line dispatcher, routes incoming server lines to registered handlers
##

from typing import Callable

HandlerFn = Callable[[str], None]

class Dispatcher:
    """Routes raw server lines to registered handler functions.

    Handlers are matched by prefix in registration order. The first
    matching handler is called. If no handler matches, the line is
    forwarded to the fallback (typically the command sender's
    on_response method).

    Attributes:
        _handlers: Ordered list of (prefix, handler) pairs.
        _fallback: Called when no prefix matches.
    """

    def __init__(self, fallback: HandlerFn) -> None:
        """Initialise the dispatcher with a fallback handler.

        Args:
            fallback: Called with any line that does not match a
                registered prefix. Typically CommandSender.on_response.
        """
        self._handlers: list[tuple[str, HandlerFn]] = []
        self._fallback = fallback

    def register(self, prefix: str, handler: HandlerFn) -> None:
        """Register a handler for lines that start with a given prefix.

        Args:
            prefix: The line prefix to match (case-sensitive).
            handler: Callable invoked with the full line when matched.
        """
        self._handlers.append((prefix, handler))

    def dispatch(self, line: str) -> None:
        """Route a line to the first matching handler, or the fallback.

        Args:
            line: A complete server line (without the trailing newline).
        """
        for prefix, handler in self._handlers:
            if line.startswith(prefix):
                handler(line)
                return
        self._fallback(line)
