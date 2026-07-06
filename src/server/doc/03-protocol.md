`libzappy_protocol` is pure parsing and validation logic, split between AI and GUI. It has no network dependency, which makes it directly unit testable.

```
libs/protocol/include/protocol/
  ai/   CommandKind.hpp  CommandInfo.hpp  CommandParser.hpp  ParsedCommand.hpp
  gui/  GuiCommandKind.hpp  GuiCommandInfo.hpp  GuiCommandParser.hpp  ParsedGuiCommand.hpp
```

## AI commands

`CommandKind` enumerates the 12 AI commands. `CommandInfo` holds the metadata table `COMMAND_INFOS`, with name, duration in time units, and whether the command takes an argument.

| Command | Cost in time units | Argument |
|---------|--------------------|----------|
| Forward, Right, Left, Look, Eject | 7 | no |
| Inventory | 1 | no |
| Broadcast | 7 | yes (text) |
| Connect_nbr | 0 (instant) | no |
| Fork | 42 | no |
| Take, Set | 7 | yes (resource) |
| Incantation | 300 | no |

Real duration is `durationTimeUnits * 1000 / frequency` milliseconds, so a larger `-f` makes the game faster.

`CommandParser::parse(line)` returns `optional<ParsedCommand>`. It trims the line, extracts the verb, and validates the presence or absence of an argument against the table. On failure it returns `nullopt`, and the server replies `ko`.

## GUI commands

`GuiCommandKind` enumerates the GUI commands: `msz`, `bct x y`, `mct`, `tna`, `ppo id`, `plv id`, `pin id`, `sgt`, `sst freq`. `GuiCommandInfo` holds the name and argument count for each.

`GuiCommandParser::parse(line)` returns `variant<ParsedGuiCommand, GuiParseError>`. It trims, tokenizes, strips the `#` prefix from id tokens, converts to integers, and checks the argument count. Errors map to server replies:
- `UNKNOWN_COMMAND` results in `suc`
- `BAD_PARAMETER` results in `sbp`