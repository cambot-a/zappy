This document covers configuration, the client lifecycle, the handshake, the scheduler, and the `Server` class that wires everything together.

## CLI and configuration

Files: `src/server/cli/`

- `CliParser::parse(argc, argv)` returns `optional<ServerConfig>`, which is empty when `--help` is given, and throws `CliParseError` on bad input. Internally `collect()` tokenizes the arguments and `finalize()` validates ranges, team name uniqueness, the reserved `GRAPHIC` name, and applies the default frequency of 100.
- `ServerConfig` is the immutable validated config container: `port()`, `width()`, `height()`, `teamNames()`, `clientsPerTeam()`, `frequency()`, plus the bonus flags.
- `include/server/Constants.hpp` holds `SUCCESS = 0`, `FAILURE = 84`, buffer sizes, and the 1 MB cap.

## Client lifecycle

Files: `src/server/client/`

The client is a one way state machine:

```
HANDSHAKE  ->  AI | GUI | GUI_ADMIN
```

- `ClientState` is the enum of the four states.
- `ClientPayloads` is a `variant<monostate, AiData{teamName, playerId}, GuiData, GuiAdminData>`.
- `Client` aggregates a `ClientBuffer`, the state, the payload, and a `markedForDrop` flag. `promote(state)` only works from `HANDSHAKE`, otherwise it throws `ClientStateError`. The typed accessors `aiData()` and `guiData()` throw if the state does not match.
- `ClientRegistry` owns all clients in an `unordered_map<int fd, Client>`. It offers `add`, `remove`, `get`, `contains`, `countInState`, `findFdByPlayerId` for the reverse lookup used to route AI responses, and `forEach` and `forEachInState`.

## Handshake

File: `src/server/handshake/HandshakeHandler.hpp`

This processes the first line of a client and returns `HandshakeResult { PROMOTED, DROP }`.

1. Trim the line. An empty line results in `ko` and `DROP`.
2. `GRAPHIC` promotes to GUI and returns `PROMOTED`.
3. Otherwise the line is treated as a team name. An unknown team gives `ko` and `DROP`. A known team triggers a draw of a waiting egg of that team:
   - no free egg gives `ko` and `DROP`
   - otherwise `spawnFromEgg` hatches the egg, spawns the player at its position with a random orientation, replies with the remaining slot count and then the world dimensions, starts food consumption, and promotes the client to `AI`.

## Scheduler and clock

Files: `src/server/scheduler/`

- `Clock.hpp` defines `IClock` and `SteadyClock` built on `steady_clock`, with `Duration` as milliseconds. The interface lets tests inject a fake clock.
- `Scheduler` is a priority queue of callbacks in a `multiset` ordered by fire time then by id.
  - `schedule(delay, cb)` and `scheduleAt(when, cb)` return an `EventId`
  - `cancel(id)` is a soft cancel that marks the event without removing it immediately
  - `tick()` runs all due, non cancelled events and never lets an exception escape, since it catches and logs to stderr
  - `rescaleDelays(factor)` recomputes remaining delays, used when the frequency changes at runtime via `sst`
  - `nextTimeoutMs()` reports the delay until the next event, used to drive the poll timeout

## The Server class

Files: `src/server/Server.cpp` and `include/server/Server.hpp`

`Server` owns every subsystem and connects them with callbacks: `Listener`, `PollLoop`, `World`, `ResourceSpawner`, `ClientRegistry`, `SteadyClock`, `Scheduler`, `RefillScheduler`, `FoodScheduler`, `AiDispatcher`, `GuiDispatcher`, `GuiNotifier`, `HandshakeHandler`.

Construction creates the listener, builds the world, seeds resources and eggs, and binds the dispatchers.

`run()` registers the listener with a callback to `acceptPendingClients`, makes stdin non blocking with a callback to `handleStdin` (stdin lines are broadcast to GUI clients as `smg`), installs the scheduler hooks, and enters `poll.run()`.

Scheduler integration:
- pre wait hook: `poll.set_next_timeout(scheduler.nextTimeoutMs())`
- post wait hook: `scheduler.tick()`

So `poll()` sleeps exactly until the next timer, then game events fire.

Per client handling:
- `acceptPendingClients` drains accepts, adds to the registry in `HANDSHAKE`, sends `WELCOME`, and registers the fd
- `handleClientEvents(fd, revents)` handles POLLIN then POLLOUT, retunes the poll mask, and applies a deferred drop if marked and there is nothing left to write
- `handlePollin` calls `on_readable` then `dispatchReadyMessages`, which routes by state: HANDSHAKE to the handshake handler, AI to `AiDispatcher::dispatch(playerId, line)`, GUI or GUI_ADMIN to `GuiDispatcher::dispatch(fd, line)`
- `dropClient(fd)` stops the AI dispatcher and food timer for an AI client and kills the player if still alive, then unregisters and removes the client
- `retunePollMask(fd)` sets `POLLIN | (has_pending_write ? POLLOUT : 0)`

Game callbacks: `onPlayerStarved` sends `dead` and drops, `onAiResponse` and `onGuiResponse` queue a reply and retune, `updateFrequency` rescales all timers and sends `sgt` to the GUI.