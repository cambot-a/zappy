# POSIX C++ Wrappers Library

This library provides type-safe, move-only RAII wrappers over core POSIX system APIs (sockets, file descriptors, address structures). It ensures that system resources are cleaned up properly and prevents typical C-style errors like file descriptor leaks.

## Exposed Components

- **`zappy::posix::FileDescriptor`**: A move-only wrapper taking ownership of a raw integer file descriptor. It automatically closes it upon destruction using RAII.
- **`zappy::posix::Address`**: A copyable wrapper over `sockaddr_in` for IPv4 sockets. It simplifies address construction, validation, and manipulation.

## Usage Example

Here is an example showing how to create and bind a local socket using the library:

```cpp
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include "posix/FileDescriptor.hpp"
#include "posix/Address.hpp"

/**
 * @brief Initialize and bind a local socket.
 *
 * @param ip IPv4 address string
 * @param port port number
 * @return zappy::posix::FileDescriptor the bound socket file descriptor
 */
zappy::posix::FileDescriptor bindLocalSocket(
    const std::string &ip, std::uint16_t port)
{
    const int rawFd = socket(AF_INET, SOCK_STREAM, 0);

    if (rawFd < 0) {
        throw std::runtime_error("failed to create socket");
    }
    zappy::posix::FileDescriptor mySock(rawFd);
    const zappy::posix::Address addr(ip, port);

    if (bind(mySock.get(), addr.raw_generic(), addr.size()) < 0) {
        throw std::runtime_error("failed to bind socket");
    }
    return mySock;
}
```
