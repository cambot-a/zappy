This document covers the two lowest libraries: `libzappy_posix` and `libzappy_net`. Nothing here knows about the game. The server registers callbacks on top of this engine.

## libzappy_posix

RAII ownership of system resources, so no file descriptor ever leaks.

### FileDescriptor

File: `libs/posix/include/posix/FileDescriptor.hpp`

Move only owner of a POSIX file descriptor. The destructor closes it automatically.

Public API:
- `FileDescriptor()` invalid descriptor
- `FileDescriptor(int fd)` take ownership
- `get()` access without releasing
- `release()` hand ownership to the caller
- `close()` idempotent close
- `is_valid()` and `operator bool()`

### Address

File: `libs/posix/include/posix/Address.hpp`

Copyable wrapper over an IPv4 `sockaddr_in`.

Public API:
- `Address(ip, port)`, `Address(sockaddr_in)`, `Address::any(port)`
- `ip()`, `port()`, `raw()`, `raw_generic()`, `size()`

## libzappy_net

Single threaded, non blocking network engine built on `poll()`.

### Listener

File: `libs/net/include/net/Listener.hpp`

Move only non blocking listening TCP socket. It creates a `SOCK_STREAM | SOCK_NONBLOCK` socket, sets `SO_REUSEADDR`, binds to `INADDR_ANY:port`, and listens.

Public API:
- `fd()` raw descriptor to register in the poll loop
- `port()` resolves the actually bound port through `getsockname()`, which matters when port 0 is requested
- `accept_connection()` returns `optional<FileDescriptor>`, empty on `EAGAIN`
- `peer_address(client)` static helper returning the peer `Address`

It throws `NetworkError` on any POSIX failure.

### PollLoop

File: `libs/net/include/net/PollLoop.hpp`

The event loop. It maps each fd to a callback `void(short revents)` and dispatches readiness events.

Public API:
- `register_fd(fd, events, callback)`, `unregister_fd(fd)`, `modify_events(fd, events)`
- `set_pre_wait_hook(hook)` runs before `poll()`, `set_post_wait_hook(hook)` runs after
- `set_next_timeout(ms)` sets the timeout of the next `poll()`, with -1 meaning infinite
- `run()` enters the loop until `stop()` or SIGINT
- `stop()` requests a stop in a thread safe and signal safe way

It owns a `SignalHandler` for SIGINT and a wake `eventfd`. The pre and post wait hooks are the integration point for the scheduler (see Server core).

### ClientBuffer

File: `libs/net/include/net/ClientBuffer.hpp`

Per client I/O buffering.

Buffer model: this is a linear, dynamically growing buffer, not a circular (ring) buffer. There is no fixed capacity reused with wrapping head and tail indices. On the read side a `std::string` grows with `append`, and consumed lines are removed from the front with `erase(0, pos)`, which shifts the remaining bytes. On the write side a `std::deque<std::string>` holds the pending messages, with one current buffer and a `write_offset` that advances through it. Ready inbound lines are stored in a `std::queue`. Memory grows on demand, bounded only by the 1 MB read cap.

Reading: reads in 4096 byte chunks, accumulates, and splits on `\n` into ready messages. The accumulated read buffer is capped at 1 MB, beyond which it returns `ERROR`.

Writing: messages are queued with `queue_message`, which appends `\n`, and drained progressively by `on_writable`.

Public API:
- `on_readable()` returns `ReadResult { OK, PEER_CLOSED, ERROR }`
- `on_writable()` returns `WriteResult { OK, ERROR, ALL_SENT }`
- `queue_message(msg)`, `has_pending_write()`, `pop_ready_message()`, `has_ready_messages()`

`has_pending_write()` is what drives whether `POLLOUT` should be active for that fd.

### SignalHandler

File: `libs/net/include/net/SignalHandler.hpp`

Converts SIGINT into a pollable `signalfd`. API: `read_fd()`, `consume()`. There is no global variable. This is why Ctrl+C is just another event in the loop.

### NetworkError

File: `libs/net/include/net/NetworkError.hpp`

Typed exception (`std::runtime_error`) for failures in this layer.