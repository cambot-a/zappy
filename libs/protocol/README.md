# Protocol Library

This library handles serialization and parsing of the communication protocol verbs and arguments for Zappy client-server interaction. It contains protocol specifications for both the GUI client (C++) and the AI client (Python).

## Exposed C++ Components

### GUI Namespace (`zappy::protocol::gui`)
- **`zappy::protocol::gui::GuiCommandParser`**: A stateless parser class exposing the static `parse` method. It parses incoming command lines and performs validation on argument type, counts, and non-negativity.
- **`zappy::protocol::gui::GuiCommandKind`**: Enum class containing all valid commands (`MSZ`, `BCT`, `MCT`, `TNA`, `PPO`, `PLV`, `PIN`, `SGT`, `SST`).
- **`zappy::protocol::gui::ParsedGuiCommand`**: Encapsulates a successfully parsed command category and its numerical arguments.

### AI Namespace (`zappy::protocol::ai`)
- **`zappy::protocol::ai::CommandParser`**: Stateless parser to check and extract AI player actions sent to the server.
- **`zappy::protocol::ai::CommandKind`**: Enum representing the 12 AI actions (e.g. `FORWARD`, `RIGHT`, `LEFT`, `LOOK`, `INVENTORY`, etc.).
- **`zappy::protocol::ai::ParsedCommand`**: Holds the command kind and raw string argument (e.g. object name for `Take` or `Set`, broadcast text).

---

## Usage Examples

### 1. Server-Side Parsing of GUI Commands

```cpp
#include <iostream>
#include <stdexcept>
#include <variant>
#include "protocol/gui/GuiCommandParser.hpp"

/**
 * @brief Parse and print a GUI command.
 *
 * @param line raw command line to parse
 */
void processGuiInput(std::string_view line)
{
    const auto result = zappy::protocol::gui::GuiCommandParser::parse(line);

    if (std::holds_alternative<zappy::protocol::gui::GuiParseError>(result)) {
        throw std::runtime_error("failed to parse GUI command");
    }
    const auto &cmd =
        std::get<zappy::protocol::gui::ParsedGuiCommand>(result);
    std::cout << "Command kind: " << static_cast<int>(cmd.kind) << "\n";
}
```

### 2. Python AI Client-Side Protocol (Python)
Since the AI client is written in Python, here is a Python example illustrating how the AI client formats and sends protocol actions to the server:

```python
import socket

class ZappyAiClient:
    """
    Handles connection and protocol communication for the Python Zappy AI.
    """

    def __init__(self, host: str, port: int):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.connect((host, port))
        # Consume the initial "WELCOME\n" response
        self.receive_line()

    def send_action(self, action: str, argument: str = "") -> None:
        """
        Formats and sends a command matching the Zappy AI protocol.
        Example: send_action("Take", "food") -> sends "Take food\n"
        """
        payload = f"{action} {argument}\n" if argument else f"{action}\n"
        self._socket.sendall(payload.encode("utf-8"))

    def receive_line(self) -> str:
        """
        Reads a single newline-terminated response line from the server.
        """
        data = b""
        while not data.endswith(b"\n"):
            chunk = self._socket.recv(1)
            if not chunk:
                break
            data += chunk
        return data.decode("utf-8").strip()
```
