# Net Library

This library provides core TCP networking classes, buffering, and poll-based loop abstractions. It handles TCP connection acceptance, non-blocking asynchronous socket communication, and buffering of incoming/outgoing messages.

## Exposed Components

- **`zappy::net::Listener`**: A move-only non-blocking TCP server listener that binds to a port and yields accepted client sockets as `zappy::posix::FileDescriptor` objects.
- **`zappy::net::ClientBuffer`**: Manages the socket level reading/writing and queues message lines separating them by newlines (`\n`).
- **`zappy::net::PollLoop`**: Abstraction over the POSIX `poll()` system call, allowing file descriptors to be registered and queried for readability and writability.

## Usage Example

Here is an example showing how to setup a TCP listener and accept a connection:

```cpp
#include <iostream>
#include <stdexcept>
#include "net/Listener.hpp"
#include "posix/FileDescriptor.hpp"

/**
 * @brief Accept and log incoming client connections.
 *
 * @param port server port to listen on
 */
void runSimpleServer(std::uint16_t port)
{
    zappy::net::Listener listener(port);
    std::optional<zappy::posix::FileDescriptor> clientSock;

    while (!clientSock) {
        clientSock = listener.accept_connection();
    }
    const zappy::posix::Address peer =
        zappy::net::Listener::peer_address(*clientSock);
    std::cout << "Peer connected: " << peer.ip() << ":"
        << peer.port() << "\n";
}
```
