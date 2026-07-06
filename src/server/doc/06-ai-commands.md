Files: `src/server/ai/`

## AiDispatcher

`AiDispatcher` manages, per player, a command queue (a `deque` capped at 10 pending commands), the time scheduling of those commands, and their execution.

- `dispatch(playerId, line)` parses the line through `libzappy_protocol`, replies `ko` on invalid input, otherwise enqueues. A command of duration 0 (`Connect_nbr`) runs immediately, the others are scheduled through the `Scheduler`.
- `stopPlayer(playerId)` cancels the in flight command and drops the queue.
- `setFrequency(frequency)` updates the time scale.

Execution handlers:
- `executeForward`, `executeRight`, `executeLeft` reply `ok`
- `executeLook` builds the tile list through `LookResponseBuilder`
- `executeInventory` replies with `[food n, linemate n, ...]`
- `executeTake` and `executeSet` reply `ok` or `ko`
- `executeBroadcast` replies `ok` and notifies observers
- `executeConnectNbr` replies with the number of waiting eggs
- `executeFork` replies `ok` and creates a new egg
- `executeEject` replies `ok`

## Incantation flow

This is the special multi step case:

1. On initiation the dispatcher gathers all alive players on the same tile whose level is at least L, then checks the required player count and stones. A failure replies `ko` immediately.
2. On success it freezes every participant with `freezePlayer`, sends `Elevation underway` to each, notifies the GUI, and schedules the end after 300 time units.
3. At the end it re-validates, since some participants may have died. On success `applySuccess` elevates the players and consumes the stones, otherwise `applyFailure` replies `ko`. Either way it unfreezes the participants and resumes their queues.

## LookResponseBuilder

`LookResponseBuilder::buildFor(playerId)` builds the string `[tile1, tile2, ...]`, where each tile lists players as `player` and resources by name, separated by spaces.