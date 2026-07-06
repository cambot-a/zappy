Files: `src/server/gui/`

## GuiDispatcher

`GuiDispatcher` parses GUI commands and replies. Handlers cover `msz`, `bct`, `mct` (all tiles), `tna`, `ppo`, `plv`, `pin`, `sgt`, and `sst` which changes the frequency.

`sendInitialSync(fd)` runs when a GUI client connects and sends, in order: `msz`, then every `bct`, then `tna`, then players as `pnw`, `plv`, `pin`, then eggs as `enw`, then `sgt`.

## GuiLineBuilder

`GuiLineBuilder` centralizes the construction of GUI protocol lines.

State queries: `bct`, `ppo`, `plv`, `pin`, `pnw`.

Event lines:
- `pex` player ejected
- `pbc` player broadcast
- `pfk` fork started
- `pgt` and `pdr` resource picked up and dropped
- `pdi` player died
- `pic` and `pie` incantation start and end
- `enw`, `ebo`, `edi` egg new, hatched, and dead
- `seg` end of game
- `smg` server message

## GuiNotifier

`GuiNotifier` implements `IWorldObserver` and broadcasts the matching lines to every GUI and GUI_ADMIN client through `GuiLineBuilder`. It is the bridge from the simulation to the GUI: each world mutation becomes a protocol line.

It also detects victory. When a player reaches level 8, if 6 or more players of the same team are level 8, it emits `seg <team>` once.
